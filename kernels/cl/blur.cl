#if   defined(USE_C4)
typedef float4 pix_t;
#elif defined(USE_C3)
typedef float3 pix_t;
#elif defined(USE_C2)
typedef float2 pix_t;
#elif defined(USE_C1)
typedef float  pix_t;
#else
typedef float4 pix_t;
#endif

// image_width - 1
#ifndef USE_IMAGE_W
#define USE_IMAGE_W (1U)
#endif

// image_height - 1
#ifndef USE_IMAGE_H
#define USE_IMAGE_H (1U)
#endif

// conv. kernel half size
#ifndef USE_KERN_H_S
#define USE_KERN_H_S (1U)
#endif

#ifdef USE_RESIZE
__constant sampler_t sampler =
        CLK_NORMALIZED_COORDS_TRUE |
        CLK_ADDRESS_CLAMP_TO_EDGE  |
    #ifdef USE_RESIZE_LIN
        CLK_FILTER_LINEAR;
    #else
        CLK_FILTER_NEAREST;
    #endif
#endif

inline pix_t __attribute__((always_inline)) sample_pix(
    __read_only image2d_t input_bgra,
    const       int2      pos_pix
) {
    #ifdef USE_RESIZE
    const float2 pixel_c = convert_float2(pos_pix) + (float2) (0.5f, 0.5f);
    const float2 coord_f = pixel_c / (float2) ((USE_IMAGE_W + 1), (USE_IMAGE_H + 1));
    return read_imagef(input_bgra, sampler, coord_f);
    #else
    return read_imagef(input_bgra, pos_pix);
    #endif
}

__kernel void kernel_blur_horizontal(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    pix_t sum = 0.f;

    #pragma unroll
    for (int k = -(USE_KERN_H_S); k <= (USE_KERN_H_S); ++k) {
        const float weight  = gaussian_kernel[k + (int) USE_KERN_H_S];
        const int2  pos_pix = (int2) (clamp((int) coord.x + k, (int) 0, (int) USE_IMAGE_W), coord.y);
        const pix_t pixel   = sample_pix(input_bgra, pos_pix);

        sum += (pixel * weight);
    }

    write_imagef(output_bgra, coord, sum);
}

__kernel void kernel_blur_vertical(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    pix_t sum = 0.f;

    #pragma unroll
    for (int k = -(USE_KERN_H_S); k <= (USE_KERN_H_S); ++k) {
        const float weight  = gaussian_kernel[k + (int) USE_KERN_H_S];
        const int2  pos_pix = (int2) (coord.x, clamp((int) coord.y + k, (int) 0, (int) USE_IMAGE_H));
        const pix_t pixel   = read_imagef(input_bgra, pos_pix); // not using sampler here

        sum += (pixel * weight);
    }

    write_imagef(output_bgra, coord, sum);
}

__kernel void kernel_blur_single(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    pix_t sum = 0.f;

    #pragma unroll
    for (int i = 0, y = -(USE_KERN_H_S); y <= (USE_KERN_H_S); ++y) {
        #pragma unroll
        for (int x = -(USE_KERN_H_S); x <= (USE_KERN_H_S); ++x) {
            int2        pos_pix = clamp(coord + ((int2) (x, y)), (int2) (0), (int2) (USE_IMAGE_W, USE_IMAGE_H));
            const pix_t pixel   = sample_pix(input_bgra, pos_pix);
            const float weight  = gaussian_kernel[i++];
            sum += (pixel * weight);
        }
    }

    write_imagef(output_bgra, coord, sum);
}