//
// Created by xd on 4/14/25.
//

#ifndef EX_MOTION_CL_IMAGE_H
#define EX_MOTION_CL_IMAGE_H

#include <CL/cl.h>

namespace ex::ocl {

    using uint32 = u_int32_t;
    using uint16 = u_int16_t;
    using uchar8 = u_int8_t;

    // 16 bytes
    struct Image2D {

        enum class ACCESS: uchar8 {
            /** READ ONLY */
            RO = 1,

            /** WRITE ONLY */
            WO = RO << 1,

            /** READ WRITE */
            RW = RO | WO
        };

        struct header {
            uchar8 cn_depth : 3;
            uchar8 channels : 2;
            ACCESS access   : 2;
        };

        cl_mem handle;
        uint16 id;
        uint16 cols;
        uint16 rows;
        header header;

        uchar8 ____; // reserved for internal usage

        uint32 size() const;

        bool empty() const;

        bool detached() const;

        static Image2D copy(const Image2D &source);

        static Image2D detached(::size_t cols,
                                ::size_t rows,
                                ::size_t channels,
                                ::size_t channel_size,
                                ACCESS access = ACCESS::RW);

        static void allocate(Image2D &img,
                             cl_context context);

        static Image2D allocate(::size_t cols,
                                ::size_t rows,
                                ::size_t channels,
                                ::size_t channel_size,
                                cl_context context,
                                ACCESS access = ACCESS::RW);

        static Image2D allocate_like(const Image2D &img,
                                     cl_context context,
                                     ACCESS access = ACCESS::RW);

        static void release(Image2D &img);
    };

    // 24 bytes
    struct ImagePromise {

        cl_command_queue queue;
        Image2D          image;

        ImagePromise(const Image2D &image, cl_command_queue queue, bool ready);

        ImagePromise(const Image2D &image, cl_command_queue queue);

        ImagePromise(const Image2D &image);

        void set_completed(bool completed);

        bool completed() const;

        // blocking op
        void finalize(bool force = false);

        // blocking op
        static void finalize_all(ImagePromise *arr, int n, bool force = false);
    };

    // 16 bytes
    struct Promise {

        cl_command_queue queue;
        bool             ready;

        Promise(cl_command_queue queue, bool ready);

        Promise(cl_command_queue queue);

        Promise(const ImagePromise &promise);

        bool completed() const;

        // blocking op
        void finalize(bool force = false);

        // blocking op
        static void finalize_all(Promise *arr, int n, bool force = false);
    };

    namespace ops {

        // non blocking op
        ImagePromise from_bytes(
            const unsigned char *bytes,
            ::size_t width,
            ::size_t height,
            ::size_t channel_n,
            ::size_t channel_size,
            cl_context context,
            cl_command_queue queue,
            Image2D::ACCESS access = Image2D::ACCESS::RW
        );

        // non blocking op
        Promise to_bytes(const Image2D &image, unsigned char *out, cl_command_queue queue);

        // non blocking op
        Promise to_bytes(ImagePromise &promise, unsigned char *out);

    }

}

#endif //EX_MOTION_CL_IMAGE_H
