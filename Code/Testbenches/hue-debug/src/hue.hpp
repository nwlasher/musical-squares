/* This file defines the data structures, constants, and functions to be
   implemented in hue.cpp. This module will provide the color to sound
   mapping that will drive the toy's function */

#pragma once
#include "TCS3472x.hpp"
#include "synth.hpp"

// Hue is in degrees from 0.0 to 270.0. Red->Violet
const float HUE_VIOLET = 270.0f;
const float HUE_MAGENTA_SPLIT = 315.0f;
const float HUE_RED = 0.0f;

// Returns the hue given a Color struct and a saturation
float hueFromColor(const Color &c, float &sat);
// 0.0 = lowest tone (violet), 1.0 = highest (red)
// loose clamp from 315, placing hue at whichever end is closer
// should reduce jitter on red/purple
float hueToPosition(float hue);
// Same as above but clamped to violet in the 270-360 range
// Discrete mapping for a given hue to one of the finite tones 
Tone hueToTone(float hue);
// Continuous mapping from hue to hz
float hueToHz(float hue);

