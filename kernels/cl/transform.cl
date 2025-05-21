#define USE_ROTATE
#define USE_FLIP
#define USE_FLIP_X
#define USE_FLIP_Y
#define USE_DOWNSCALE
#define USE_DOWNSCALE_LIN


#ifdef USE_ROTATE
inline void rotate(const ushort2 size_xy, int *pix_x, int *pix_y) {
    const int tmp = *pix_x;
    *pix_x = size_xy.y - (*pix_y) - 1;
    *pix_y = tmp;
}
#endif

#ifdef USE_FLIP
inline void flip(const ushort2 size_xy, int *pix_x, int *pix_y) {
    #ifdef USE_FLIP_X
    *pix_x = size_xy.x - *pix_x - 1;
    #endif
    #ifdef USE_FLIP_Y
    *pix_y = size_xy.y - *pix_y - 1;
    #endif
}
#endif

#ifdef USE_DOWNSCALE
inline void downscale(
    __global const uchar4 *in_img,     // from
                   uchar4 *out_pix,    // to
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

__kernel void kernel_transform(
    __global const uchar4 *input,
    __global       uchar4 *output,
             const ushort2 size_img,
             const ushort2 size_og
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= size_img.x || y >= size_img.y)
        return;

    #ifdef USE_ROTATE
    rotate(size_img,  &x, &y);
    #endif

    #ifdef USE_FLIP
    flip(size_img, &x, &y);
    #endif
    // TODO
}