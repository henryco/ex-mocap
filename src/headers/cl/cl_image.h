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

    using image2d_t = Image2D;
    using promise_t = Promise;
    using img_promise = ImagePromise;

    using uint32 = u_int32_t;
    using uint16 = u_int16_t;
    using uchar8 = u_int8_t;

    enum class C_MODE: uchar8 {
        A    = 0U,
        R    = 1U,
        RG   = 2U,
        RA   = 3U,
        RGB  = 4U, // not sure about this
        RGBA = 5U,
        BGRA = 6U,
        ARGB = 7U,
    };

    enum class C_DEPT: uchar8 {
        UINT_N8  = 0U, // normalized uchar
        UINT_N16 = 1U, // normalized ushort
        UINT_8   = 2U, // uchar
        UINT_16  = 3U, // ushort
        UINT_32  = 4U, // uint
        FLOAT_16 = 5U, // half-float
        FLOAT_32 = 6U, // full-float
    };

    // number of channels
    unsigned channel_count(C_MODE cn_mode);

    // size of the channel in bytes
    unsigned channel_size(C_DEPT cn_dept);

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
            C_MODE channels : 3;
            C_DEPT cn_depth : 3;
            ACCESS access   : 2; // 00/RO/WO/RW
            // -------------
            uchar8 fresh    : 1; // 0/1
            uchar8 ready    : 1; // 0/1
            uchar8 ____2    : 1; // reserved for internal usage
            uchar8 ____3    : 1; // reserved for internal usage
            uchar8 ____4    : 1; // reserved for internal usage
            uchar8 ____5    : 1; // reserved for internal usage
            uchar8 ____6    : 1; // reserved for internal usage
            uchar8 ____7    : 1; // reserved for internal usage
            // -------------
        };

        cl_mem handle;
        uint16 id;
        uint16 cols;
        uint16 rows;
        header header;

        uint32 size() const;

        bool empty() const;

        bool detached() const;

        void release();

        static Image2D blank();

        static Image2D detached(::size_t cols,
                                ::size_t rows,
                                C_MODE channels,
                                C_DEPT channel_size,
                                ACCESS access = ACCESS::RW);

        static void allocate(Image2D &img,
                             cl_context context);

        static Image2D allocate(::size_t cols,
                                ::size_t rows,
                                C_MODE channels,
                                C_DEPT channel_size,
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

        NBPromise(cl_command_queue, cl_event);

        NBPromise(const ImagePromise &);

        NBPromise(const Promise &);

        NBPromise();

        void release();

        bool completed();

        bool empty() const;

        // blocking op
        void finalize();

        // blocking op
        static void finalize_all(NBPromise *arr, int n);
    };

    namespace ops {

        // non blocking op
        ImagePromise from_bytes(
            const unsigned char *bytes,
            ::size_t width,
            ::size_t height,
            C_MODE channel_n,
            C_DEPT channel_size,
            cl_context context,
            cl_command_queue queue,
            Image2D::ACCESS access = Image2D::ACCESS::RW
        );

        // non blocking op
        ImagePromise from_bytes(
            const unsigned char *bytes,
            const Image2D &image,
            cl_context context,
            cl_command_queue queue,
            Image2D::ACCESS access = Image2D::ACCESS::RW
        );

        // non blocking op
        void from_bytes(const unsigned char *bytes, Image2D &image_dst, cl_command_queue);

        // non blocking op
        void from_bytes(const unsigned char *bytes, ImagePromise &image_dst);

        // non blocking op
        Promise to_bytes(const Image2D &, void *out, cl_command_queue);

        // non blocking op
        Promise to_bytes(const ImagePromise &, void *out);

        // non blocking op
        ImagePromise copy_data(const Image2D &, cl_context, cl_command_queue);

        // non blocking op
        ImagePromise copy_data(const ImagePromise &, cl_context);

        // non blocking op
        Promise copy_data(const ImagePromise &promise_src, Image2D &image_dst);

        // non blocking op
        Promise copy_data(const Image2D &image_src, Image2D &image_dst, cl_command_queue);

        // non blocking op
        NBPromise barrier(const NBPromise &, cl_command_queue);

        // non blocking op
        NBPromise barrier(const NBPromise &);

        // non blocking op
        NBPromise barrier(cl_command_queue);
    }

}

#endif //EX_MOTION_CL_IMAGE_H
