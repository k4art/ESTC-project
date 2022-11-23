#include "color_picker/colors.h"

// works only when hsv components are ranged in [0..255]
// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
rgb_color_t hsv_to_rgb(hsv_color_t hsv)
{
  rgb_color_t rgb;

  uint8_t region, remainder, p, q, t;

  if (hsv.saturation == 0)
  {
    return (rgb_color_t) RGB_COLOR(hsv.value, hsv.value, hsv.value);
  }

  region = hsv.hue / 43;
  remainder = (hsv.hue - (region * 43)) * 6;

  p = (hsv.value * (255 - hsv.saturation)) >> 8;
  q = (hsv.value * (255 - ((hsv.saturation * remainder) >> 8))) >> 8;
  t = (hsv.value * (255 - ((hsv.saturation * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
    case 0: return (rgb_color_t) RGB_COLOR(hsv.value, t, p);
    case 1: return (rgb_color_t) RGB_COLOR(q, hsv.value, p);
    case 2: return (rgb_color_t) RGB_COLOR(p, hsv.value, t);
    case 3: return (rgb_color_t) RGB_COLOR(p, q, hsv.value);
    case 4: return (rgb_color_t) RGB_COLOR(t, p, hsv.value);
    case 5: return (rgb_color_t) RGB_COLOR(hsv.value, p, q);

    default:
      NRFX_ASSERT(false);
  }

  return rgb;
}

