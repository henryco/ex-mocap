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
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x >= USE_IMAGE_W || coord.y >= USE_IMAGE_H)
        return;

    const float4 pixel = read_imagef(input_bgra, coord);

    float  weight_sum = 0.f;
    float4 sum        = 0.f;

    // TODO

    sum /= weight_sum;
    sum.w = 1.;
    write_imagef(output_bgra, coord, sum);
}

__kernel void kernel_blur_vertical(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x >= USE_IMAGE_W || coord.y >= USE_IMAGE_H)
        return;

    const float4 pixel = read_imagef(input_bgra, coord);

    float  weight_sum = 0.f;
    float4 sum        = 0.f;

    // TODO

    sum /= weight_sum;
    sum.w = 1.;
    write_imagef(output_bgra, coord, sum);
}

__kernel void kernel_blur_single(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x >= USE_IMAGE_W || coord.y >= USE_IMAGE_H)
        return;

    const float4 pixel = read_imagef(input_bgra, coord);

    float  weight_sum = 0.f;
    float4 sum        = 0.f;

    // TODO

    sum /= weight_sum;
    sum.w = 1.;
    write_imagef(output_bgra, coord, sum);
}