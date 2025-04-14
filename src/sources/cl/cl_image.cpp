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

    Image2D Image2D::copy(const Image2D &source) {
        Image2D another_one = source;
        return another_one;
    }

    Image2D Image2D::detached(const ::size_t cols, const ::size_t rows, const ::size_t channels,
                              const ::size_t channel_size, const ACCESS access) {
        return { .handle = nullptr,
                 .id     = 0,
                 .cols   = static_cast<uint16>(cols),
                 .rows   = static_cast<uint16>(rows),
                 .header = { .cn_depth = static_cast<uchar8>(channel_size),
                             .channels = static_cast<uchar8>(channels),
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

    void ImagePromise::set_completed(const bool completed) {
        image.____ = completed;
    }

    bool ImagePromise::completed() const {
        return image.____ == true;
    }

    void ImagePromise::finalize(const bool force) {
        if ((image.____ == true && !force) || !queue)
            return;
        if (cl_int err = clFinish(queue); err != CL_SUCCESS)
            abort__("Cannot finish command queue: %d", err);
    }

    void ImagePromise::finalize_all(ImagePromise *const promises, const int size, const bool force) {
        thread_local cl_command_queue list[256];
        int n = 0;

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

} // namespace ex::ocl
