#ifndef USE_IMAGE_W
#define USE_IMAGE_W (1U)
#endif

#ifndef USE_IMAGE_H
#define USE_IMAGE_H (1U)
#endif

#ifndef USE_KERN_H_S
#define USE_KERN_H_S (1U)
#endif

__kernel void kernel_blur_horizontal(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    // TODO
}

__kernel void kernel_blur_vertical(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    // TODO
}

__kernel void kernel_blur_single(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    // TODO
}