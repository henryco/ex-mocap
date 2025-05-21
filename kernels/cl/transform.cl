// MANDATORY PARAMETERS:
// - USE_DIMENSION_X (ushort)
// - USE_DIMENSION_Y (ushort)
// - USE_IW          (ushort)
// - USE_IH          (ushort)
// - USE_OW          (ushort)

// ROTATION PARAMETERS:
// - USE_ROTATE

// FLIP PARAMETERS:
// - USE_FLIP_X
// - USE_FLIP_Y

// CROP PARAMETERS:
// - USE_TRANSLATE_X (optional)
// - USE_TRANSLATE_Y (optional)

// DOWNSCALE PARAMETERS:
// - USE_DOWNSCALE
// - USE_DOWNSCALE_LIN (optional)
// - USE_SX            (float)
// - USE_SY            (float)

// UPSCALE PARAMETERS:
// - USE_UPSCALE
// - USE_SX (float)
// - USE_SY (float)
// - USE_CX (ushort)
// - USE_CY (ushort)
// - USE_UW (ushort)
// - USE_UH (ushort)

// width of working image
#ifndef USE_DIMENSION_X
#define USE_DIMENSION_X (1U)
#endif

// height of working image
#ifndef USE_DIMENSION_Y
#define USE_DIMENSION_Y (1U)
#endif

// input image width
#ifndef USE_IW
#define USE_IW (1U)
#endif

// input image height
#ifndef USE_IH
#define USE_IH (1U)
#endif

// output image width
#ifndef USE_OW
#define USE_OW (1U)
#endif

// scale width
#ifndef USE_SX
#define USE_SX (1.f)
#endif

// scale height
#ifndef USE_SY
#define USE_SY (1.f)
#endif

// ceil(scale width)
#ifndef USE_CX
#define USE_CX (1U)
#endif

// ceil(scale height)
#ifndef USE_CY
#define USE_CY (1U)
#endif

// upscale width
#ifndef USE_UW
#define USE_UW (1U)
#endif

// upscale height
#ifndef USE_UH
#define USE_UH (1U)
#endif

//#define USE_ROTATE
//#define USE_FLIP_X
//#define USE_FLIP_Y
//#define USE_UPSCALE
//#define USE_DOWNSCALE
//#define USE_DOWNSCALE_LIN

#ifdef USE_DOWNSCALE
inline void downscale(
    __global const uchar4 *in_img,     // from
    __global       uchar4 *out_pix,    // to
             const ushort img_w,       // from
             const ushort img_h,       // from
             const float  scale_w,     // [from : to], ie: [2 : 1]
             const float  scale_h,     // [from : to], ie: [2 : 1]
             const int    pix_x,       // to
             const int    pix_y        // to
) {
    #ifndef USE_DOWNSCALE_LIN

    const int img_x = clamp((int) (pix_x * scale_w), (int) 0, (int) (img_w - 1));
    const int img_y = clamp((int) (pix_y * scale_h), (int) 0, (int) (img_h - 1));
    const int pos   = ((img_y * img_w) + img_x);
    out_pix[0] = in_img[pos];

    #else

    const float img_x = pix_x * scale_w;
    const float img_y = pix_y * scale_h;

    const int x0   = (int) img_x;
    const int y0   = (int) img_y;
    const float dx = img_x - x0;
    const float dy = img_y - y0;

    const int x1 = min((int) (x0 + 1), (int) (img_w - 1));
    const int y1 = min((int) (y0 + 1), (int) (img_h - 1));

    // Load 4 source pixels as uchar4 and convert to float4
    const float4 p00 = convert_float4(in_img[y0 * img_w + x0]);
    const float4 p10 = convert_float4(in_img[y0 * img_w + x1]);
    const float4 p01 = convert_float4(in_img[y1 * img_w + x0]);
    const float4 p11 = convert_float4(in_img[y1 * img_w + x1]);

    // Interpolate horizontally
    const float4 p0 = p00 + (p10 - p00) * dx;
    const float4 p1 = p01 + (p11 - p01) * dx;

    // Interpolate vertically
    const float4 result = p0 + (p1 - p0) * dy;

    // Store the result as uchar4
    uchar4 out = convert_uchar4_sat_rte(result);
    out.w = 255;
    out_pix[0] = out;

    #endif
}
#endif

#ifdef USE_UPSCALE
inline void upscale(
    __global       uchar4 *out_pix,      // pixels to
             const uchar4 color,         // pixel (one) from
             const ushort output_w,      // to width
             const ushort output_h,      // to height
             const float  scale_w,       // [to : from], ie: [2 : 1]
             const float  scale_h,       // [to : from], ie: [2 : 1]
             const float  ceil_scale_w,  // ceil(scale_w)
             const float  ceil_scale_h,  // ceil(scale_H)
             const int    pix_x,         // to
             const int    pix_y          // to
) {

    const float img_x = pix_x * scale_w;
    const float img_y = pix_y * scale_h;

    for (int ky = 0; ky < ceil_scale_h; ky++) {
        for (int kx = 0; kx < ceil_scale_w; kx++) {
            const int ix    = clamp((int) (img_x + kx), (int) 0, (int) (output_w - 1));
            const int iy    = clamp((int) (img_y + ky), (int) 0, (int) (output_h - 1));
            const int o_idx = (iy * output_w + ix);
            out_pix[o_idx] = color;
        }
    }
}
#endif

__kernel void kernel_transform(
    __global const uchar4 *input_bgra,
    __global       uchar4 *output_bgra
) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    #ifdef USE_ROTATE
    if (x >= USE_DIMENSION_Y || y >= USE_DIMENSION_X)
        return; // rotated dimension
    const uint og_x = x;
    const uint og_y = y;
    {
        const uint tmp = y;
        y = USE_DIMENSION_X - x - 1;
        x = tmp;
    }
    #else
    if (x >= USE_DIMENSION_X || y >= USE_DIMENSION_Y)
        return;
    const uint og_x = x;
    const uint og_y = y;
    #endif

    #ifdef USE_FLIP_X
    x = USE_DIMENSION_X - x - 1;
    #endif

    #ifdef USE_FLIP_Y
    y = USE_DIMENSION_Y - y - 1;
    #endif

    #ifdef USE_TRANSLATE_X
    x += USE_TRANSLATE_X;
    #endif

    #ifdef USE_TRANSLATE_Y
    y += USE_TRANSLATE_Y;
    #endif

    #if defined(USE_DOWNSCALE)
    downscale(input_bgra,
              &output_bgra[(og_y * USE_OW) + og_x],
              USE_IW,
              USE_IH,
              USE_SX,
              USE_SY,
              x,
              y);
    #elif defined(USE_UPSCALE)
    upscale(output_bgra,
            input_bgra[(y * USE_IW) + x],
            USE_UW,
            USE_UH,
            USE_SX,
            USE_SY,
            USE_CX,
            USE_CY,
            og_x,
            og_y);
    #else
    output_bgra[(og_y * USE_OW) + og_x] = input_bgra[(y * USE_IW) + x];
    #endif
}