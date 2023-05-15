#include "lib/utils.h"

#include "colors.h"

const uint8_t HSV_COMPONENT_TOP_VALUES[HSV_COMPONENTS_NUMBER] =
{
  H_COMPONENT_TOP_VALUE,
  S_COMPONENT_TOP_VALUE,
  V_COMPONENT_TOP_VALUE,
};

/*
 * rgb_to_hsv and hsv_to_rgb are adopted from (by @leszek-szary)
 * https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
 */

hsv_color_t rgb_to_hsv(rgb_color_t rgb)
{
  uint8_t h, s, v;
  uint8_t rgb_min, rgb_max;

  rgb_min = rgb.red < rgb.green ? (rgb.red < rgb.blue ? rgb.red : rgb.blue) : (rgb.green < rgb.blue ? rgb.green : rgb.blue);
  rgb_max = rgb.red > rgb.green ? (rgb.red > rgb.blue ? rgb.red : rgb.blue) : (rgb.green > rgb.blue ? rgb.green : rgb.blue);

  v = rgb_max;

  if (v == 0)
  {
    return HSV_COLOR(0, 0, 0);
  }

  s = NRFX_ROUNDED_DIV(255 * (rgb_max - rgb_min), v);

  if (s == 0)
  {
    return HSV_COLOR(0, 0, v);
  }

  if (rgb_max == rgb.red)
  {
    h = 0 + NRFX_ROUNDED_DIV(43 * (rgb.green - rgb.blue), (rgb_max - rgb_min));
  }
  else if (rgb_max == rgb.green)
  {
    h = 85 + 43 * NRFX_ROUNDED_DIV((rgb.blue - rgb.red), (rgb_max - rgb_min));
  }
  else
  {
    h = 171 + 43 * NRFX_ROUNDED_DIV((rgb.red - rgb.green), (rgb_max - rgb_min));
  }

  return HSV_COLOR(h, s, v);
}

/*
 * This implementation assumes that the following conditions are true.
 */
STATIC_ASSERT(H_COMPONENT_TOP_VALUE == 255);
STATIC_ASSERT(S_COMPONENT_TOP_VALUE == 255);
STATIC_ASSERT(V_COMPONENT_TOP_VALUE == 255);

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

hsv_color_t word_to_hsv(uint32_t word)
{
  uint8_t h = (word >>  0) & 0xFF;
  uint8_t s = (word >>  8) & 0xFF;
  uint8_t v = (word >> 16) & 0xFF;

  return HSV_COLOR(h, s, v);
}

uint32_t hsv_to_word(hsv_color_t hsv)
{
  return (hsv.hue >> 0) | (hsv.saturation << 8) | (hsv.value << 16);
}

uint32_t rgb_to_word(rgb_color_t rgb)
{
  return (rgb.red >> 0) | (rgb.green << 8) | (rgb.blue << 16);
}

rgb_color_t word_to_rgb(uint32_t word)
{
  uint8_t r = (word >>  0) & 0xFF;
  uint8_t g = (word >>  8) & 0xFF;
  uint8_t b = (word >> 16) & 0xFF;

  return RGB_COLOR(r, g, b);
}

