//
// Created by xd on 27/11/24.
//

#include "../../headers/cl/cl_gl_interop.h"
#include "../../headers/utils/log_utils.h"

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3native.h>
#include <CL/cl_gl.h>

namespace ex::cl::utils {

    ex::logs::logger provide_logger() {
        static const auto log = ex::logs::get_logger("cl_gl_interop");
        return log;
    }

    cl_context_properties* get_cl_gl_context_properties(GLFWwindow* window, cl_platform_id platform) {
        const auto log = provide_logger();
        log->info("Getting cl_gl context properties");

        static cl_context_properties properties[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
#if defined(_WIN32)
            CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetWGLContext(window),
            CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
#elif defined(__linux__)
            CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetGLXContext(window),
            CL_GLX_DISPLAY_KHR, (cl_context_properties) glfwGetX11Display(),
#endif
            0
        };

        return properties;
    }

    cl_context create_cl_gl_context(GLFWwindow* window) {
        const auto log = provide_logger();

        log->info("Creating cl_gl context");

        cl_platform_id platform;
        cl_int         err = clGetPlatformIDs(1, &platform, nullptr);
        if (err != CL_SUCCESS) {
            log->error("Failed to get cl platform ids: {}", err);
            return nullptr;
        }

        cl_device_id device;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
        if (err != CL_SUCCESS) {
            log->error("Failed to get opencl device id: {}", err);
            return nullptr;
        }

        auto props = ex::cl::utils::get_cl_gl_context_properties(window, platform);
        log->info("Acquired cl_gl interop context properties");

        cl_context context = clCreateContext(props, 1, &device,
                                             nullptr, nullptr, &err);
        if (err != CL_SUCCESS) {
            log->error("Failed to create opencl context: {}", err);
            return nullptr;
        }

        log->info("Created cl_gl interop context: {}", err == CL_SUCCESS);
        return context;
    }

    void check_gl_sharing_support() {
        const auto log = provide_logger();

        // Get the number of platforms
        cl_uint numPlatforms;
        cl_int  err = clGetPlatformIDs(0, nullptr, &numPlatforms);
        if (err != CL_SUCCESS) {
            log->error("Failed to get number of OpenCL platforms: {}", err);
            return;
        }

        // Get the platform IDs
        std::vector<cl_platform_id> platforms(numPlatforms);
        err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
        if (err != CL_SUCCESS) {
            log->error("Failed to get OpenCL platforms: {}", err);
            return;
        }

        // Iterate over each platform
        for (cl_uint i = 0; i < numPlatforms; ++i) {
            // Get the number of devices for this platform
            cl_uint numDevices;
            err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
            if (err != CL_SUCCESS) {
                log->error("Failed to get OpenCL devices: {}", err);
                continue;
            }

            // Get the device IDs
            std::vector<cl_device_id> devices(numDevices);
            err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr);
            if (err != CL_SUCCESS) {
                log->error("Failed to get OpenCL devices: {}", err);
                continue;
            }

            // Iterate over each device
            for (cl_uint j = 0; j < numDevices; ++j) {
                // Get the extensions string size
                size_t extensionSize;
                err = clGetDeviceInfo(devices[j], CL_DEVICE_EXTENSIONS, 0, nullptr, &extensionSize);
                if (err != CL_SUCCESS) {
                    log->warn("Failed to get OpenCL device extension size: {}", err);
                    continue;
                }

                // Get the extensions string
                std::vector<char> extensions(extensionSize);
                err = clGetDeviceInfo(devices[j], CL_DEVICE_EXTENSIONS, extensionSize, extensions.data(), nullptr);
                if (err != CL_SUCCESS) {
                    log->warn("Failed to get OpenCL device extensions: {}", err);
                    continue;
                }

                // Check if cl_khr_gl_sharing is supported
                std::string extensionsStr(extensions.begin(), extensions.end());
                if (extensionsStr.find("cl_khr_gl_sharing") != std::string::npos) {
                    log->info("Device: [{}] on Platform: [{}] supports cl_khr_gl_sharing", j, i);
                } else {
                    log->warn("Device: [{}] on Platform: [{}] does not supports cl_khr_gl_sharing", j, i);
                }
            }
        }
    }
}

