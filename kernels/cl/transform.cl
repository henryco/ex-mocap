// MANDATORY PARAMETERS:
// - USE_DIMENSION_X (ushort)
// - USE_DIMENSION_Y (ushort)

// ROTATION PARAMETERS:
// - USE_ROTATE

// FLIP PARAMETERS:
// - USE_FLIP_X
// - USE_FLIP_Y

// CROP PARAMETERS:
// - USE_TRANSLATE_X (optional)
// - USE_TRANSLATE_Y (optional)

// RESIZE PARAMETERS:
// - USE_RESIZE
// - USE_RESIZE_LIN

// width of working image
#ifndef USE_DIMENSION_X
#define USE_DIMENSION_X (1U)
#endif

// height of working image
#ifndef USE_DIMENSION_Y
#define USE_DIMENSION_Y (1U)
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

__kernel void kernel_transform(
    read_only  image2d_t input_bgra,
    write_only image2d_t output_bgra
) {
    int2       coord     = (int2) (get_global_id(0), get_global_id(1));
    const int2 dst_coord = (int2) (coord.x, coord.y);

    #ifdef USE_ROTATE
    if (coord.x >= USE_DIMENSION_Y || coord.y >= USE_DIMENSION_X)
        return; // rotated dimension
    const uint tmp = coord.y;
    coord.y = USE_DIMENSION_Y - coord.x - 1;
    coord.x = tmp;
    #else
    if (coord.x >= USE_DIMENSION_X || coord.y >= USE_DIMENSION_Y)
        return;
    #endif

    #ifdef USE_FLIP_X
    coord.x = USE_DIMENSION_X - coord.x - 1;
    #endif

    #ifdef USE_FLIP_Y
    coord.y = USE_DIMENSION_Y - coord.y - 1;
    #endif

    #ifdef USE_TRANSLATE_X
    coord.x += USE_TRANSLATE_X;
    #endif

    #ifdef USE_TRANSLATE_Y
    coord.y += USE_TRANSLATE_Y;
    #endif

    #ifdef USE_RESIZE
    const float2 pixel_c = convert_float2(coord) + (float2)(0.5f, 0.5f);
    const float2 coord_f = pixel_c / (float2)((USE_DIMENSION_X), (USE_DIMENSION_Y));
    const float4 pixel   = read_imagef(input_bgra, sampler, coord_f);
    #else
    const float4 pixel = read_imagef(input_bgra, coord);
    #endif

    write_imagef(output_bgra, dst_coord, pixel);
}