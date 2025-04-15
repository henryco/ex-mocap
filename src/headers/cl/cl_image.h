//
// Created by xd on 4/14/25.
//

#ifndef EX_MOTION_CL_IMAGE_H
#define EX_MOTION_CL_IMAGE_H

#include <CL/cl.h>

namespace ex::ocl {

    struct Image2D;
    struct Promise;
    struct NBPromise;
    struct ImagePromise;

    using image2d_ptr = Image2D *;
    using promise_ptr = Promise *;
    using img_promise = ImagePromise *;

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
            uchar8 cn_depth : 3; // 0-8
            uchar8 channels : 3; // 0/1/3/4
            ACCESS access   : 2; // 00/RO/WO/RW
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

        void release();

        static Image2D blank();

        /**
         * <b> THIS WILL NOT COPY DATA UNDER CL_MEM HANDLE! </b>
         */
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

        ImagePromise();

        void set_completed(bool completed);

        bool completed() const;

        bool empty() const;

        Promise raw_promise() const;

        NBPromise non_blocking() const;

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

        Promise();

        bool completed() const;

        bool empty() const;

        NBPromise non_blocking() const;

        // blocking op
        void finalize(bool force = false);

        // blocking op
        static void finalize_all(Promise *arr, int n, bool force = false);
    };

    // 16 bytes
    struct NBPromise {

        cl_command_queue queue;
        cl_event         marker;

        NBPromise(const ImagePromise &promise);

        NBPromise(const Promise &promise);

        NBPromise();

        void release();

        bool completed();

        bool empty() const;

        // blocking op
        void finalize(bool force = false);

        // blocking op
        static void finalize_all(NBPromise *arr, int n, bool force = false);
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
        ImagePromise from_bytes(
            const unsigned char *bytes,
            const Image2D &image,
            cl_command_queue queue
        );

        // non blocking op
        ImagePromise from_bytes(
            const unsigned char *bytes,
            const ImagePromise &image
        );

        // non blocking op
        Promise to_bytes(const Image2D &image, unsigned char *out, cl_command_queue queue);

        // non blocking op
        Promise to_bytes(const ImagePromise &promise, unsigned char *out);

        // non blocking op
        ImagePromise copy_data(const Image2D &image, cl_context context, cl_command_queue queue);

        // non blocking op
        ImagePromise copy_data(const ImagePromise &promise, cl_context context);

        // non blocking op
        Promise copy_data(const ImagePromise &promise_src, Image2D &image_dst);

        // non blocking op
        Promise copy_data(const Image2D &image_src, Image2D &image_dst, cl_command_queue queue);

    }

}

#endif //EX_MOTION_CL_IMAGE_H
