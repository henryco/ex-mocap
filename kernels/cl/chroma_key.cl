
// if (lower_h < upper_h) { // Dont wrap
//        if (hls[0] >= lower_h &&
//            hls[1] >= lower_l &&
//            hls[2] >= lower_s &&
//            hls[0] <= upper_h &&
//            hls[1] <= upper_l &&
//            hls[2] <= upper_s) {
//            return 255;
//        } else {
//            return 0;
//        }
//    }
//
//    else { // WRAP !
//        if ((hls[0] >= lower_h || hls[0] <= upper_h) &&
//            hls[1] >= lower_l &&
//            hls[2] >= lower_s &&
//            hls[1] <= upper_l &&
//            hls[2] <= upper_s) {
//            return 255;
//        } else {
//            return 0;
//        }
//    }


void bgr_to_hls(const float4 *in_bgra, float4 *out_hlsa) {
    const float r = in_bgra->x;
    const float g = in_bgra->y;
    const float b = in_bgra->z;

    const float c_max = fmax(r, fmax(g, b));
    const float c_min = fmin(r, fmin(g, b));
    const float c_dif = c_max - c_min;
    const float c_sum = c_max + c_min;

    const float eps      = 1e-10f;
    const float safe_dif = fmax(c_dif, eps);
    const float L = 0.5f * c_sum;

    const float Hf = (c_max == r) * ((g - b) / safe_dif) +
                     (c_max == g) * ((b - r) / safe_dif + 2.f) +
                     (c_max == b) * ((r - g) / safe_dif + 4.f);

    float H = 60.f * Hf;
    if (H < 0.f) H += 360.f;

    float S = 0.f;
    if (c_max != c_min) {
        const float denominator = (L < 0.5f) ? c_sum : (2.f - c_sum);
        S = c_dif / fmax(denominator, eps);
    }

    *out_hlsa = (float4) (H / 360.f, L, S, 1.f);
}



