#ifndef __COLORS_H
#define __COLORS_H

#include "nrfx.h"

/********** RGB color **********/

#define RGB_COMPONENTS_NUMBER 3

#define RGB_COMPONENTS_TOP_VALUE 255

#define R_COMPONENT_IDX 0
#define G_COMPONENT_IDX 1
#define B_COMPONENT_IDX 2

typedef struct rgb_color_s
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
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
  uint8_t hue;
  uint8_t saturation;
  uint8_t value;
} hsv_color_t;

#define HSV_COLOR(h, s, v) (hsv_color_t) { .hue = h, .saturation = s, .value = v }

/* The opposite convertation is not provided. */
rgb_color_t hsv_to_rgb(hsv_color_t hsv);

#endif
