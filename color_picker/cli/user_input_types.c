#include "nrfx.h"

#include "user_input_types.h"

int USER_RGB_COLOR_MAXS[RGB_COMPONENTS_NUMBER] =
{
  RGB_COMPONENTS_TOP_VALUE,
  RGB_COMPONENTS_TOP_VALUE,
  RGB_COMPONENTS_TOP_VALUE,
};

int USER_HSV_COLOR_MAXS[HSV_COMPONENTS_NUMBER] =
{
  [H_COMPONENT_IDX] = 360,
  [S_COMPONENT_IDX] = 100,
  [V_COMPONENT_IDX] = 100,
};

rgb_color_t rgb_color_from_user_rgb(const user_rgb_color_t * p_user_rgb)
{
  rgb_color_t rgb;

  for (size_t i = 0; i < RGB_COMPONENTS_NUMBER; i++)
  {
    rgb.components[i] = p_user_rgb->components[i];
  }

  return rgb;
}

hsv_color_t hsv_color_from_user_hsv(const user_hsv_color_t * p_user_hsv)
{
  hsv_color_t hsv;

  for (size_t i = 0; i < HSV_COMPONENTS_NUMBER; i++)
  {
    int user_value    = p_user_hsv->components[i];
    int user_max      = USER_HSV_COLOR_MAXS[i];

    uint8_t top_value = HSV_COMPONENT_TOP_VALUES[i];

    hsv.components[i] = NRFX_ROUNDED_DIV((unsigned int) user_value * top_value, user_max);
  }

  return hsv;
}
