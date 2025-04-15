//
// Created by xd on 4/14/25.
//

#include "../../headers/cl/cl_image.h"
#include "../../headers/utils/err_utils.h"

namespace ex::ocl {

    uint32 Image2D::size() const {
        return cols * rows * header.channels * header.cn_depth;
    }

    bool Image2D::empty() const {
        return size() == 0;
    }

    bool Image2D::detached() const {
        return handle == nullptr;
    }

    void Image2D::release() {
        if (handle == nullptr)
            return;
        clReleaseMemObject(handle);
        handle = nullptr;
    }

    Image2D Image2D::blank() {
        return { .handle = nullptr,
                 .id     = 0,
                 .cols   = static_cast<uint16>(0),
                 .rows   = static_cast<uint16>(0),
                 .header = { .cn_depth = static_cast<uchar8>(0),
                             .channels = static_cast<uchar8>(0),
                             .access   = ACCESS::RW } };
    }

    Image2D Image2D::copy(const Image2D &source) {
        Image2D another_one = source;
        return another_one;
    }

    Image2D Image2D::detached(const ::size_t cols, const ::size_t rows, const ::size_t channels,
                              const ::size_t channel_size, const ACCESS access) {
        const uchar8 cs = static_cast<uchar8>(channel_size);
        const uchar8 cn = static_cast<uchar8>(channels);
        return { .handle = nullptr,
                 .id     = 0,
                 .cols   = static_cast<uint16>(cols),
                 .rows   = static_cast<uint16>(rows),
                 .header = { .cn_depth = cs,
                             .channels = cn,
                             .access   = access } };
    }

    void Image2D::allocate(Image2D &img, const cl_context context) {
        if (img.handle != nullptr)
            abort__("Image has been bound to cl_mem object already!");

        cl_mem_flags flags;
        if (img.header.access == ACCESS::RO) flags = CL_MEM_READ_ONLY;
        else if (img.header.access == ACCESS::WO) flags = CL_MEM_WRITE_ONLY;
        else if (img.header.access == ACCESS::RW) flags = CL_MEM_READ_WRITE;
        else abort__("Invalid access modifier: %d", static_cast<int>(img.header.access));

        cl_int err;
        cl_mem buffer = clCreateBuffer(context, flags, img.size(), nullptr, &err);
        if (err != CL_SUCCESS)
            abort__("Cannot create cl buffer: %d", err);

        img.handle = buffer;
    }

    Image2D Image2D::allocate(const ::size_t cols, const ::size_t rows, const ::size_t channels,
                              const ::size_t channel_size, const cl_context context, const ACCESS access) {
        Image2D image = detached(cols, rows, channels, channel_size, access);
        allocate(image, context);
        return image;
    }

    Image2D Image2D::allocate_like(const Image2D &t, const cl_context context, const ACCESS access) {
        Image2D copy       = Image2D::copy(t);
        copy.handle        = nullptr;
        copy.header.access = access;
        allocate(copy, context);
        return copy;
    }

    void Image2D::release(Image2D &img) {
        if (img.handle == nullptr)
            return;
        clReleaseMemObject(img.handle);
        img.handle = nullptr;
    }

    Promise::Promise(const cl_command_queue queue):
        queue(queue), ready(false) {
    }

    Promise::Promise(const cl_command_queue queue, const bool ready):
        queue(queue), ready(ready) {
    }

    Promise::Promise(const ImagePromise &promise) {
        this->queue = promise.queue;
        this->ready = promise.image.____;
    }

    Promise::Promise():
        queue(nullptr), ready(false) {
    }

    bool Promise::completed() const {
        return ready;
    }

    bool Promise::empty() const {
        return queue == nullptr;
    }

    NBPromise Promise::non_blocking() const {
        return *this;
    }

    void Promise::finalize(const bool force) {
        if ((ready && !force) || !queue)
            return;
        if (cl_int err = clFinish(queue); err != CL_SUCCESS)
            abort__("Cannot finish command queue: %d", err);
        ready = true;
    }

    void Promise::finalize_all(Promise *const promises, const int size, const bool force) {
        thread_local cl_command_queue list[256];
        int                           n = 0;

        for (int g = 0; g < size; g++) {
            auto &p = promises[g];

            if (p.ready && !force)
                continue;

            p.ready = true;

            bool found = false;
            for (int i = 0; i < n; i++) {
                if (list[i] != p.queue) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                list[n++] = p.queue;

                if (cl_int err = clFinish(p.queue); err != CL_SUCCESS)
                    abort__("Cannot finish command queue: %d", err);
            }
        }
    }

    NBPromise::NBPromise(const ImagePromise &promise) {
        queue = promise.queue;
        marker = nullptr;

        if (queue == nullptr)
            return;

        if (cl_int err = clEnqueueMarkerWithWaitList(queue,
            0,
            nullptr,
            &marker);
            err != CL_SUCCESS) {
            abort__("clEnqueueMarkerWithWaitList abort");
        }
    }

    NBPromise::NBPromise(const Promise &promise) {
        queue = promise.queue;
        marker = nullptr;

        if (queue == nullptr)
            return;

        if (cl_int err = clEnqueueMarkerWithWaitList(queue,
            0,
            nullptr,
            &marker);
            err != CL_SUCCESS) {
            abort__("clEnqueueMarkerWithWaitList abort");
        }
    }

    NBPromise::NBPromise():
        queue(nullptr),
        marker(nullptr) {
    }

    NBPromise::~NBPromise() {
        if (marker == nullptr)
            return;
        clReleaseEvent(marker);
        marker = nullptr;
    }

    bool NBPromise::completed() {
        if (marker == nullptr)
            return true;

        cl_int status;

        if (cl_int err = clGetEventInfo(marker, CL_EVENT_COMMAND_EXECUTION_STATUS,
            sizeof(status), &status, nullptr);
            err != CL_SUCCESS) {
            abort__("clGetEventInfo abort");
        }

        if (status == CL_COMPLETE) {
            clReleaseEvent(marker);
            marker = nullptr;
            return true;
        }

        return false;
    }

    bool NBPromise::empty() const {
        return queue == nullptr && marker == nullptr;
    }

    void NBPromise::finalize(const bool force) {
        if ((marker == nullptr && !force) || !queue)
            return;
        if (cl_int err = clFinish(queue); err != CL_SUCCESS)
            abort__("Cannot finish command queue: %d", err);

        if (marker == nullptr)
            return;
        clReleaseEvent(marker);
        marker = nullptr;
    }

    void NBPromise::finalize_all(NBPromise *promises, const int size, const bool force) {
        thread_local cl_command_queue list[256];
        int                           n = 0;

        for (int g = 0; g < size; g++) {
            auto &p = promises[g];

            if (p.marker == nullptr && !force)
                continue;

            if (p.marker != nullptr) {
                clReleaseEvent(p.marker);
                p.marker = nullptr;
            }

            bool found = false;
            for (int i = 0; i < n; i++) {
                if (list[i] != p.queue) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                list[n++] = p.queue;

                if (cl_int err = clFinish(p.queue); err != CL_SUCCESS)
                    abort__("Cannot finish command queue: %d", err);
            }
        }
    }

    ImagePromise::ImagePromise(const Image2D &image, const cl_command_queue queue, const bool ready):
        queue(queue), image(image) {
        this->image.____ = ready;
    }

    ImagePromise::ImagePromise(const Image2D &image, const cl_command_queue queue):
        queue(queue), image(image) {
        this->image.____ = false;
    }

    ImagePromise::ImagePromise(const Image2D &image) {
        this->queue      = nullptr;
        this->image      = image;
        this->image.____ = true;
    }

    ImagePromise::ImagePromise() {
        this->queue      = nullptr;
        this->image      = Image2D::blank();
        this->image.____ = false;
    }

    void ImagePromise::set_completed(const bool completed) {
        image.____ = completed;
    }

    bool ImagePromise::completed() const {
        return image.____ == true;
    }

    bool ImagePromise::empty() const {
        return queue == nullptr && image.empty();
    }

    NBPromise ImagePromise::non_blocking() const {
        return *this;
    }

    void ImagePromise::finalize(const bool force) {
        if ((image.____ == true && !force) || !queue)
            return;
        if (cl_int err = clFinish(queue); err != CL_SUCCESS)
            abort__("Cannot finish command queue: %d", err);
        image.____ = true;
    }

    void ImagePromise::finalize_all(ImagePromise *const promises, const int size, const bool force) {
        thread_local cl_command_queue list[256];
        int                           n = 0;

        for (int g = 0; g < size; g++) {
            auto &p = promises[g];

            if (p.image.____ == true && !force)
                continue;

            p.image.____ = true;

            bool found = false;
            for (int i = 0; i < n; i++) {
                if (list[i] != p.queue) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                list[n++] = p.queue;

                if (cl_int err = clFinish(p.queue); err != CL_SUCCESS)
                    abort__("Cannot finish command queue: %d", err);
            }
        }
    }

    namespace ops {
        cl_mem_flags access_to_cl(const Image2D::ACCESS access) {
            if (access == Image2D::ACCESS::RW)
                return CL_MEM_READ_WRITE;
            if (access == Image2D::ACCESS::RO)
                return CL_MEM_READ_ONLY;
            if (access == Image2D::ACCESS::WO)
                return CL_MEM_WRITE_ONLY;
            abort__("Unknown access modifier");
        }

        ImagePromise from_bytes(const unsigned char *bytes, const ::size_t width, const ::size_t height,
                                const ::size_t channel_n, const ::size_t channel_size, const cl_context context,
                                const cl_command_queue queue, const Image2D::ACCESS access) {
            Image2D        image = Image2D::allocate(width, height, channel_n, channel_size, context, access);
            const ::size_t size  = image.size();

            cl_int err;

            image.handle = clCreateBuffer(context, access_to_cl(access), size, nullptr, &err);
            if (err != CL_SUCCESS)
                abort__("Cannot create cl buffer: %d", err);

            err = clEnqueueWriteBuffer(queue, image.handle, CL_FALSE, 0,
                size, bytes, 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
                abort__("Cannot enqueue copy data to cl buffer: %d", err);

            return { image, queue };
        }

        ImagePromise from_bytes(const unsigned char *bytes, Image2D image, const cl_command_queue queue) {
            if (image.handle == nullptr)
                abort__("Image cl_mem ptr is null");

            cl_int err =
                    clEnqueueWriteBuffer(queue, image.handle, CL_FALSE, 0, image.size(),
                        bytes, 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
                abort__("Cannot enqueue copy data to cl buffer: %d", err);

            return { image, queue };
        }

        ImagePromise from_bytes(const unsigned char *bytes, const ImagePromise &promise) {
            return from_bytes(bytes, promise.image, promise.queue);
        }

        Promise to_bytes(const Image2D &image, unsigned char *out, const cl_command_queue queue) {
            if (image.handle == nullptr)
                abort__("Image cl_mem ptr is null");
            cl_int err = clEnqueueReadBuffer(queue,
                                        image.handle,
                                        CL_FALSE,
                                        0,
                                        image.size(),
                                        out,
                                        0,
                                        nullptr,
                                        nullptr);
            if (err != CL_SUCCESS)
                abort__("Cannot enqueue read buffer: %d", err);
            return queue;
        }

        Promise to_bytes(ImagePromise &promise, unsigned char *out) {
            return to_bytes(promise.image, out, promise.queue);
        }

    }

}
