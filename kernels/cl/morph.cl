// image_width - 1
#ifndef USE_IMAGE_W
#define USE_IMAGE_W (1U)
#endif

// image_height - 1
#ifndef USE_IMAGE_H
#define USE_IMAGE_H (1U)
#endif

// erode. kernel half size
#ifndef USE_KERN_H_S_E
#define USE_KERN_H_S_E (1U)
#endif

// dilate. kernel half size
#ifndef USE_KERN_H_S_D
#define USE_KERN_H_S_D (1U)
#endif

__kernel void dilate_horizontal(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float4 max_val = 0.f;

    #pragma unroll
    for (int k = -(USE_KERN_H_S_D); k <= (USE_KERN_H_S_D); ++k) {
        const int2   pos_pix = (int2) (clamp((int) coord.x + k, (int) 0, (int) USE_IMAGE_W), coord.y);
        const float4 pixel   = read_imagef(input_bgra, pos_pix);
        max_val              = max(max_val, pixel);
    }

    write_imagef(output_bgra, coord, max_val);
}

__kernel void dilate_vertical(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float4 max_val = 0.f;

    #pragma unroll
    for (int k = -(USE_KERN_H_S_D); k <= (USE_KERN_H_S_D); ++k) {
        const int2   pos_pix = (int2) (coord.x, clamp((int) coord.y + k, (int) 0, (int) USE_IMAGE_H));
        const float4 pixel   = read_imagef(input_bgra, pos_pix);
        max_val              = max(max_val, pixel);
    }

    write_imagef(output_bgra, coord, max_val);
}

__kernel void erode_horizontal(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float4 min_val = 0.f;

    #pragma unroll
    for (int k = -(USE_KERN_H_S_E); k <= (USE_KERN_H_S_E); ++k) {
        const int2   pos_pix = (int2) (clamp((int) coord.x + k, (int) 0, (int) USE_IMAGE_W), coord.y);
        const float4 pixel   = read_imagef(input_bgra, pos_pix);
        min_val              = max(min_val, pixel);
    }

    write_imagef(output_bgra, coord, min_val);
}

__kernel void erode_vertical(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_bgra
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x > USE_IMAGE_W || coord.y > USE_IMAGE_H)
        return;

    float4 min_val = 0.f;

    #pragma unroll
    for (int k = -(USE_KERN_H_S_E); k <= (USE_KERN_H_S_E); ++k) {
        const int2   pos_pix = (int2) (coord.x, clamp((int) coord.y + k, (int) 0, (int) USE_IMAGE_H));
        const float4 pixel   = read_imagef(input_bgra, pos_pix);
        min_val              = min(min_val, pixel);
    }

    write_imagef(output_bgra, coord, min_val);
}