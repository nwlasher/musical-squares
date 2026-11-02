// hue.cpp
#include "hue.hpp"
#include <math.h>

// hue.cpp (add near the top, after the includes)
float hueFromColor(const Color &k, float &sat){
    float r = k.r, g = k.g, b = k.b;
    float mx = fmaxf(r, fmaxf(g, b));
    float mn = fminf(r, fminf(g, b));
    float d  = mx - mn;
    sat = (mx > 0) ? d / mx : 0;
    if (d == 0) return 0;
    float h;
    if (mx == r)      h = fmodf((g - b) / d, 6.0f);
    else if (mx == g) h = (b - r) / d + 2.0f;
    else              h = (r - g) / d + 4.0f;
    h *= 60.0f;
    return (h < 0) ? h + 360.0f : h;
}

float hueToPosition(float hue){
    // Instead of a hard clamp send hues near magenta to the edge they're closer to
    if (hue > HUE_MAGENTA_SPLIT) hue = HUE_RED;
    else if (hue > HUE_VIOLET) hue = HUE_VIOLET; 
    return (HUE_VIOLET - hue) / HUE_VIOLET;     // red -> 1, violet -> 0
}

float hueToClampedPosition(float hue){
    if (hue > HUE_VIOLET) hue = HUE_VIOLET; // Clamp at Magenta
    return (HUE_VIOLET - hue) / HUE_VIOLET;     // red -> 1, violet -> 0
}

Tone hueToTone(float hue){
    // Map a hue to a tone if it is within it's frequency band according to
    // the raw mappings. 
    int i = (int)(hueToPosition(hue) * NUM_TONES);   // N equal-width bands
    if (i > NUM_TONES - 1) i = NUM_TONES - 1;        // p == 1.0 lands here
    return (Tone)i;
}

float hueToHz(float hue){
    // Place each note at the CENTER of its band, interpolate between neighbours.
    // Linear change would not visit tones
    float pos = hueToPosition(hue) * NUM_TONES - 0.5f;
    if (pos <= 0.0f) return TONE_HZ[0];
    if (pos >= NUM_TONES - 1) return TONE_HZ[NUM_TONES - 1];
    int   i    = (int)pos;
    float frac = pos - i;
    return TONE_HZ[i] + frac * (TONE_HZ[i + 1] - TONE_HZ[i]);
}
