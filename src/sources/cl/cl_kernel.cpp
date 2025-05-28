//
// Created by henryco on 6/6/24.
//

#include "../../headers/cl/cl_kernel.h"
#include <chrono>
#include <cstring>
#include "../../headers/utils/err_utils.h"

namespace xm::ocl {

    cl_context create_context(cl_device_id device) {
        cl_int err;
        cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        if (err != CL_SUCCESS)
            abort__("Cannot create cl_context: %d", err);
        return context;
    }

    cl_device_id find_gpu_device() {
        cl_int err;

        cl_uint platform_count;
        err = clGetPlatformIDs(0, nullptr, &platform_count);
        if (err != CL_SUCCESS)
            abort__("Cannot query platforms count: %d", err);

        cl_platform_id *platforms;
        platforms = new cl_platform_id[platform_count];
        err = clGetPlatformIDs(platform_count, platforms, nullptr);
        if (err != CL_SUCCESS)
            abort__("Cannot query platforms: %d", err);

        cl_uint device_count;
        cl_platform_id platform = platforms[0];
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &device_count);
        if (err != CL_SUCCESS)
            abort__("Cannot query gpu device count: %d", err);

        cl_device_id *devices;
        devices = new cl_device_id[device_count];
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, device_count, devices, nullptr);
        if (err != CL_SUCCESS)
            abort__("Cannot query gpu device id: %d", err);

        auto dev = devices[0];
        delete[] platforms;
        delete[] devices;
        return dev;
    }

    size_t optimal_global_size(int dim, size_t local_size) {
        if (dim % local_size == 0)
            return dim;
        return dim + local_size - (dim % local_size);
    }

    size_t optimal_local_size(cl_device_id device, cl_kernel kernel) {
        cl_int err;

        size_t max_workgroup_size;
        err = clGetDeviceInfo(
                device,
                CL_DEVICE_MAX_WORK_GROUP_SIZE,
                sizeof(size_t),
                &max_workgroup_size,
                nullptr);
        if (err != CL_SUCCESS)
            abort__("Error querying device for max work-group size: %d", err);

        size_t preferred_workgroup_size_multiple;
        err = clGetKernelWorkGroupInfo(
                kernel,
                device,
                CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                sizeof(size_t),
                &preferred_workgroup_size_multiple,
                nullptr);
        if (err != CL_SUCCESS)
            abort__("Error querying kernel for preferred work-group size multiple: %d", err);

        return preferred_workgroup_size_multiple > max_workgroup_size
               ? max_workgroup_size
               : preferred_workgroup_size_multiple;
    }

    static cl_program build_program(cl_context context, cl_device_id device, const char *kernel_source, const char *name, const char *options) {

        const char *o = (options == nullptr || ::strlen(options) == 0) ? nullptr : options;
        const char *s = kernel_source;

        cl_int err;
        cl_program program = clCreateProgramWithSource(context, 1, &s, nullptr, &err);
        if (err != CL_SUCCESS)
            abort__("Cannot create cl_program from source [%s]: %d", name, err);

        err = clBuildProgram(program, 1, &device, o, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            size_t log_size;
            err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
            if (err != CL_SUCCESS)
                abort__("Error getting build log size [%s]: %d", name, err);

            auto log = new char[log_size];
            err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
            if (err != CL_SUCCESS) {
                delete[] log;
                abort__("Error getting error build log [%s]: %d", name, err);
            }
            abort__("Cannot build cl_program [%s]: %s%d", name, log, log_size);
        }

        return program;
    }

    cl_program build_program(cl_context context, cl_device_id device, const char *source, size_t source_size, const char *name, const char *options) {
        char *buffer = static_cast<char *>(::malloc(source_size + 1));
        ::strncpy(buffer, source, source_size);
        buffer[source_size] = '\0';
        cl_program program  = build_program(context, device, buffer, name, options);
        ::free(buffer);
        return program;
    }

    cl_kernel build_kernel(cl_program program, const char *name) {
        cl_int err;
        cl_kernel kernel = clCreateKernel(program, name, &err);
        if (err != CL_SUCCESS)
            abort__("Cannot build cl_kernel [%d]: %s", err, name);
        return kernel;
    }

    cl_command_queue create_queue(cl_context context, cl_device_id device, bool profile) {
        cl_int err;
        cl_command_queue_properties profiling = profile ? CL_QUEUE_PROFILING_ENABLE : 0;
        cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES, profiling, 0};
        cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);
        if (err != CL_SUCCESS)
            abort__("Cannot create cl_command_queue: %d", err);
        return queue;
    }

    cl_command_queue create_queue_device(cl_context context, cl_device_id device, bool order, bool profile) {
        cl_int err;

        cl_command_queue_properties device_queue_properties;
        err = clGetDeviceInfo(device,
                              CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES,
                              sizeof(device_queue_properties),
                              &device_queue_properties,
                              nullptr);
        if (err != CL_SUCCESS)
            abort__("Cannot query device for command queue properties: %d", err);

//        cl_command_queue_properties dev_queue = (device_queue_properties & CL_QUEUE_ON_DEVICE) ? CL_QUEUE_ON_DEVICE : 0;
        cl_command_queue_properties dev_queue = 0; // TODO TEST LATER
        cl_command_queue_properties profiling = profile ? CL_QUEUE_PROFILING_ENABLE : 0;
        cl_command_queue_properties concurrent = (!order) ? CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE : 0;
        cl_command_queue_properties properties[] = {
                CL_QUEUE_PROPERTIES, profiling | dev_queue | concurrent,
                0};
        cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);
        if (err != CL_SUCCESS)
            abort__("Cannot create cl_command_queue: %d", err);
        return queue;
    }

    cl_ulong measure_exec_time(cl_event kernel_event) {
        cl_ulong start_time, end_time;
        clGetEventProfilingInfo(
                kernel_event,
                CL_PROFILING_COMMAND_START,
                sizeof(start_time),
                &start_time,
                nullptr);
        clGetEventProfilingInfo(
                kernel_event,
                CL_PROFILING_COMMAND_END,
                sizeof(end_time),
                &end_time,
                nullptr);
        return end_time - start_time;
    }

    cl_uint set_kernel_arg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, void *arg_value) {
        const auto err = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
        if (err != CL_SUCCESS)
            abort__("Cannot set kernel argument %d:", arg_index, err);
        return arg_index + (cl_uint) 1;
    }

    cl_uint set_kernel_arg_ptr(cl_kernel kernel, cl_uint arg_index, void *svm_ptr) {
        const auto err = clSetKernelArgSVMPointer(kernel, arg_index, svm_ptr);
        if (err != CL_SUCCESS)
            abort__("Cannot set kernel argument ptr %d:", arg_index, err);
        return arg_index + (cl_uint) 1;
    }

    cl_ulong enqueue_kernel_sync(
            cl_command_queue command_queue,
            cl_kernel kernel,
            cl_uint work_dim,
            const size_t *global_work_size,
            const size_t *local_work_size,
            bool profile) {
        cl_event kernel_event;
        cl_ulong time = 0;
        cl_int err;

        err = clEnqueueNDRangeKernel(
                command_queue,
                kernel,
                work_dim,
                nullptr,
                global_work_size,
                local_work_size,
                0,
                nullptr,
                &kernel_event);
        if (err != CL_SUCCESS)
            abort__("Cannot enqueue kernel %d:", err);

        err = clFlush(command_queue);
        if (err != CL_SUCCESS)
            abort__("Cannot flush command queue %d:", err);

        clWaitForEvents(1, &kernel_event);
        if (profile)
            time = xm::ocl::measure_exec_time(kernel_event);
        clReleaseEvent(kernel_event);

        return time;
    }

    cl_event enqueue_kernel_fast(
            cl_command_queue command_queue,
            cl_kernel kernel,
            cl_uint work_dim,
            const size_t *global_work_size,
            const size_t *local_work_size,
            bool profile) {
        cl_event kernel_event;

        cl_int err;
        err = clEnqueueNDRangeKernel(
                command_queue,
                kernel,
                work_dim,
                nullptr,
                global_work_size,
                local_work_size,
                0,
                nullptr,
                profile ? &kernel_event : nullptr);
        if (err != CL_SUCCESS)
            abort__("Cannot enqueue kernel %d:", err);

        return profile ? kernel_event : nullptr;
    }

    void finish_queue(cl_command_queue queue) {
        cl_int err;
        err = clFinish(queue);
        if (err != CL_SUCCESS)
            abort__("Cannot finish command queue %d:", err);
    }

    bool check_svm_cap(cl_device_id device) {
        cl_device_svm_capabilities svmCapabilities;
        cl_int err = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(svmCapabilities), &svmCapabilities, nullptr);
        if (err != CL_SUCCESS)
            abort__("Cannot query for device svm capabilities %d:", err);
        return (svmCapabilities & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER);
    }

    void release_event(cl_event event) {
        if (event != nullptr)
            clReleaseEvent(event);
    }

    bool check_extension(const char *ext_name, cl_device_id dev_id) {
        cl_int err;

        cl_uint platform_count;
        err = clGetPlatformIDs(0, nullptr, &platform_count);
        if (err != CL_SUCCESS)
            abort__("Cannot query platforms count %d:", err);

        cl_platform_id *platforms;
        platforms = new cl_platform_id[platform_count];
        err = clGetPlatformIDs(platform_count, platforms, nullptr);
        if (err != CL_SUCCESS)
            abort__("Cannot query platforms %d:", err);

        cl_platform_id platform = platforms[0];
        delete[] platforms;

        size_t platform_ext_size;
        err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &platform_ext_size);
        if (err != CL_SUCCESS) {
            abort__("Cannot query gpu device platform extension size %d:", err);
        }

        auto platform_extensions = new char[platform_ext_size];
        err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, platform_ext_size, platform_extensions, NULL);
        if (err != CL_SUCCESS) {
            delete[] platform_extensions;
            abort__("Cannot query gpu device platform extension size: %d", err);
        }

        cl_device_id device_id;
        if (dev_id == nullptr) {
            cl_uint device_count;
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &device_count);
            if (err != CL_SUCCESS) {
                delete[] platform_extensions;
                abort__("Cannot query gpu device count: %d", err);
            }

            cl_device_id *devices;
            devices = new cl_device_id[device_count];
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, device_count, devices, nullptr);
            if (err != CL_SUCCESS) {
                delete[] platform_extensions;
                delete[] devices;
                abort__("Cannot query gpu device id: %d", err);
            }
            device_id = devices[0];
            delete[] devices;
        } else {
            device_id = dev_id;
        }

        size_t device_ext_size;
        err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, NULL, &device_ext_size);
        if (err != CL_SUCCESS) {
            delete[] platform_extensions;
            abort__("Cannot query gpu device extensions size: %d", err);
        }

        auto device_extensions = new char[device_ext_size];
        err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, device_ext_size, device_extensions, NULL);
        if (err != CL_SUCCESS) {
            delete[] platform_extensions;
            delete[] device_extensions;
            abort__("Cannot query gpu device extensions: %d", err);
        }

        const std::string p(platform_extensions, platform_ext_size);
        const std::string d(device_extensions, device_ext_size);

        const auto support = (p.find(ext_name) != std::string::npos)
                && (d.find(ext_name) != std::string::npos);

        delete[] platform_extensions;
        delete[] device_extensions;
        return support;
    }

    cl_uint get_ref_count(cl_mem obj) {
        cl_uint count;
        cl_int err = clGetMemObjectInfo(obj, CL_MEM_REFERENCE_COUNT, sizeof(count), &count, NULL);
        if (err == CL_SUCCESS)
            return count;
        abort__("Cannot query reference count for cl_mem: %d", err);
    }

    void query_image2d_max_size(cl_device_id device, size_t *max_w, size_t *max_h) {
        clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &max_w, NULL);
        clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &max_h, NULL);
    }
}