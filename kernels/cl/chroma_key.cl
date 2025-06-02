// image_width
#ifndef USE_IMAGE_W
#define USE_IMAGE_W (1U)
#endif

// image_height
#ifndef USE_IMAGE_H
#define USE_IMAGE_H (1U)
#endif

// mask width
#ifndef USE_MASK_W
#define USE_MASK_W (1U)
#endif

// mask height
#ifndef USE_MASK_H
#define USE_MASK_H (1U)
#endif

#if (USE_IMAGE_W != USE_MASK_W) || (USE_IMAGE_H != USE_MASK_H)
#define USE_RESIZE
#endif

#ifdef USE_RESIZE
__constant sampler_t sampler =
        CLK_NORMALIZED_COORDS_TRUE |
        CLK_ADDRESS_CLAMP_TO_EDGE |
    #ifdef USE_RESIZE_LIN
        CLK_FILTER_LINEAR;
    #else
        CLK_FILTER_NEAREST;
    #endif
#endif

#define EPS (1e-10f)

inline void __attribute__((always_inline)) bgr_to_hls(const float4 *in_bgra, float4 *out_hlsa) {
    // https://docs.opencv.org/4.9.0/de/d25/imgproc_color_conversions.html
    const float r = in_bgra->x;
    const float g = in_bgra->y;
    const float b = in_bgra->z;

    const float c_max = fmax(r, fmax(g, b));
    const float c_min = fmin(r, fmin(g, b));
    const float c_dif = c_max - c_min;
    const float c_sum = c_max + c_min;

    const float safe_dif = fmax(c_dif, EPS);
    const float L = 0.5f * c_sum;

    const float Hf = (c_max == r) * ((g - b) / safe_dif) +
                     (c_max == g) * ((b - r) / safe_dif + 2.f) +
                     (c_max == b) * ((r - g) / safe_dif + 4.f);

    float H = 60.f * Hf;
    if (H < 0.f) H += 360.f;

    float S = 0.f;
    if (c_max != c_min) {
        const float denominator = (L < 0.5f) ? c_sum : (2.f - c_sum);
        S = c_dif / fmax(denominator, EPS);
    }

    *out_hlsa = (float4) (H / 360.f, L, S, 1.f);
}

inline float __attribute__((always_inline)) hls_in_range(const float4 hlsa, const float4 hls_lo, const float4 hls_hi) {
    // dont wrap hue
    if (hls_lo.x < hls_hi.x) {
        if (all(hlsa.xyz >= hls_lo.xyz) && all(hlsa.xyz <= hls_hi.xyz))
            return 1.f;
        return 0.f;
    }
    // wrap hue
    if ((hlsa.x >= hls_lo.x || hlsa.x <= hls_hi.x) &&
        all(hlsa.yz >= hls_lo.yz) &&
        all(hlsa.yz <= hls_hi.yz)) {
        return 1.f;
    }
    return 0.f;
}

__kernel void chroma_key_mask(
    __read_only  image2d_t input_bgra,
    __write_only image2d_t output_mask,   // possibly smaller then input
    const        float4    hls_lo,
    const        float4    hls_hi
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x >= USE_MASK_W || coord.y >= USE_MASK_H)
        return;

    #ifdef USE_RESIZE
    const float2 pixel_c = convert_float2(coord) + (float2) (0.5f, 0.5f);
    const float2 coord_f = pixel_c / (float2)((USE_MASK_W), (USE_MASK_H));
    const float4 pixel   = read_imagef(input_bgra, sampler, coord_f);
    #else
    const float4 pixel = read_imagef(input_bgra, coord);
    #endif

    float4 hlsa;
    bgr_to_hls(&pixel, &hlsa);

    const float mask = hls_in_range(hlsa, hls_lo, hls_hi);
    write_imagef(output_mask, coord, (float4) mask);
}

__kernel void chroma_mask_apply(
    __read_only  image2d_t input_bgra,
    __read_only  image2d_t input_mask,
    __write_only image2d_t output_bgra,
    const        float4    replace_bgra
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x >= USE_IMAGE_W || coord.y >= USE_IMAGE_H)
        return;

    #ifdef USE_RESIZE
    const float2 pixel_c = convert_float2(coord) + (float2) (0.5f, 0.5f);
    const float2 coord_f = pixel_c / (float2) ((USE_IMAGE_W), (USE_IMAGE_H));
    if (read_imagef(input_mask, sampler, coord_f).x > 0) {
        write_imagef(output_bgra, coord, replace_bgra);
        return;
    }
    #else
    if (read_imagef(input_mask, coord).x > 0) {
        write_imagef(output_bgra, coord, replace_bgra);
        return;
    }
    #endif

    const float4 color = read_imagef(input_bgra, coord);
    write_imagef(output_bgra, coord, color);
}

__kernel void chroma_mask_single(
    __read_only  image2d_t input_bgra,  // should be the same size
    __write_only image2d_t output_bgra, // should be the same size
    const        float4    replace_bgra,
    const        float4    hls_lo,
    const        float4    hls_hi
) {
    const int2 coord = (int2) (get_global_id(0), get_global_id(1));
    if (coord.x >= USE_IMAGE_W || coord.y >= USE_IMAGE_H)
        return;

    const float4 pixel = read_imagef(input_bgra, coord);

    float4 hlsa;
    bgr_to_hls(&pixel, &hlsa);

    const float mask = hls_in_range(hlsa, hls_lo, hls_hi);
    if (mask > 0) {
        write_imagef(output_bgra, coord, replace_bgra);
        return;
    }

    write_imagef(output_bgra, coord, pixel);
}