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

__kernel void kernel_blur_horizontal(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra,
    __constant   float    *gaussian_kernel
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float  weight_sum = 0.f;
    float4 sum        = 0.f;

    for (int k = -(USE_KERN_H_S); k <= (USE_KERN_H_S); ++k) {
        const float  weight  = gaussian_kernel[k + USE_KERN_H_S];
        const int2   pos_pix = (int2) (clamp((int) coord.x + k, (int) 0, (int) USE_IMAGE_W), coord.y);
        const float4 pixel   = read_imagef(input_bgra, pos_pix);

        sum += (pixel * weight);
        weight_sum += weight;
    }

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
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float  weight_sum = 0.f;
    float4 sum        = 0.f;

    for (int k = -(USE_KERN_H_S); k <= (USE_KERN_H_S); ++k) {
        const float  weight  = gaussian_kernel[k + USE_KERN_H_S];
        const int2   pos_pix = (int2) (coord.x, clamp((int) coord.y + k, (int) 0, (int) USE_IMAGE_H));
        const float4 pixel   = read_imagef(input_bgra, pos_pix);

        sum += (pixel * weight);
        weight_sum += weight;
    }

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
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float  weight_sum = 0.f;
    float4 sum        = 0.f;

    for (int i = 0, y = -(USE_KERN_H_S); y <= (USE_KERN_H_S); ++y) {
        for (int x = -(USE_KERN_H_S); x <= (USE_KERN_H_S); ++x) {
            const int2   pos_pix = (int2) (clamp((int) coord.x + x, (int) 0, (int) USE_IMAGE_W),
                                           clamp((int) coord.y + y, (int) 0, (int) USE_IMAGE_H));
            const float4 pixel   = read_imagef(input_bgra, pos_pix);
            const float  weight  = gaussian_kernel[i++];

            sum += (pixel * weight);
            weight_sum += weight;
        }
    }

    sum /= weight_sum;
    sum.w = 1.;
    write_imagef(output_bgra, coord, sum);
}