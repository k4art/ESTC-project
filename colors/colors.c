#include "utils.h"

#include "colors.h"

const uint8_t HSV_COMPONENT_TOP_VALUES[HSV_COMPONENTS_NUMBER] =
{
  H_COMPONENT_TOP_VALUE,
  S_COMPONENT_TOP_VALUE,
  V_COMPONENT_TOP_VALUE,
};

/*
 * This implementation assumes that the following conditions are true.
 */
STATIC_ASSERT(H_COMPONENT_TOP_VALUE == 255);
STATIC_ASSERT(S_COMPONENT_TOP_VALUE == 255);
STATIC_ASSERT(V_COMPONENT_TOP_VALUE == 255);

/*
 * Adopted from
 * https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
 */
rgb_color_t hsv_to_rgb(hsv_color_t hsv)
{
  if (hsv.saturation == 0)
  {
    return RGB_COLOR(hsv.value, hsv.value, hsv.value);
  }

  uint8_t region    = hsv.hue / 43;
  uint8_t remainder = (hsv.hue - (region * 43)) * 6;

  uint8_t p = (hsv.value * (255 - hsv.saturation)) >> 8;
  uint8_t q = (hsv.value * (255 - ((hsv.saturation * remainder) >> 8))) >> 8;
  uint8_t t = (hsv.value * (255 - ((hsv.saturation * (255 - remainder)) >> 8))) >> 8;

  NRFX_ASSERT(region < 6);

  switch (region)
  {
    case 0: return RGB_COLOR(hsv.value, t, p);
    case 1: return RGB_COLOR(q, hsv.value, p);
    case 2: return RGB_COLOR(p, hsv.value, t);
    case 3: return RGB_COLOR(p, q, hsv.value);
    case 4: return RGB_COLOR(t, p, hsv.value);
    case 5: return RGB_COLOR(hsv.value, p, q);
  }

  UNREACHABLE_RETURN(RGB_COLOR(0, 0, 0));
}
