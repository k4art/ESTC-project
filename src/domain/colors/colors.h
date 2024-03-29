#ifndef COLORS_H__
#define COLORS_H__

#include "nrfx.h"

/********** RGB color **********/

#define RGB_COMPONENTS_NUMBER 3

#define RGB_COMPONENTS_TOP_VALUE 255

typedef struct rgb_color_s
{
  union
  {
    struct
    {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
    };

    uint8_t components[RGB_COMPONENTS_NUMBER];
  };
} rgb_color_t;

#define RGB_COLOR(r, g, b) (rgb_color_t) { .red = r, .green = g, .blue = b }

/********** HSV color **********/

#define HSV_COMPONENTS_NUMBER 3

#define H_COMPONENT_TOP_VALUE 255
#define S_COMPONENT_TOP_VALUE 255
#define V_COMPONENT_TOP_VALUE 255

#define H_COMPONENT_IDX 0
#define S_COMPONENT_IDX 1
#define V_COMPONENT_IDX 2

typedef struct hsv_color_s
{
  union
  {
    struct
    {
      uint8_t hue;
      uint8_t saturation;
      uint8_t value;
    };

    uint8_t components[HSV_COMPONENTS_NUMBER];
  };
} hsv_color_t;

extern const uint8_t HSV_COMPONENT_TOP_VALUES[HSV_COMPONENTS_NUMBER];

#define HSV_COLOR(h, s, v) (hsv_color_t) { .hue = h, .saturation = s, .value = v }

hsv_color_t rgb_to_hsv(rgb_color_t rgb);
rgb_color_t hsv_to_rgb(hsv_color_t hsv);

uint32_t    rgb_to_word(rgb_color_t rgb);
rgb_color_t word_to_rgb(uint32_t    word);

uint32_t    hsv_to_word(hsv_color_t hsv);
hsv_color_t word_to_hsv(uint32_t    word);

#endif
