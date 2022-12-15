#ifndef USER_INPUT_TYPES_DEFS_H
#define USER_INPUT_TYPES_DEFS_H

#include "colors/colors.h"

/*************** USER_RGB ***************/

typedef struct user_rgb_color_s
{
  union
  {
    struct
    {
      int red;
      int green;
      int blue;
    };

    int components[RGB_COMPONENTS_NUMBER];
  };
} user_rgb_color_t;

extern int USER_RGB_COLOR_MAXS[RGB_COMPONENTS_NUMBER];

rgb_color_t rgb_color_from_user_rgb(const user_rgb_color_t * user_rgb);

/*************** USER_HSV ***************/

typedef struct user_hsv_color_s
{
  union
  {
    struct
    {
      int hue;
      int saturation;
      int value;
    };

    int components[HSV_COMPONENTS_NUMBER];
  };
} user_hsv_color_t;

extern int USER_HSV_COLOR_MAXS[HSV_COMPONENTS_NUMBER];

hsv_color_t hsv_color_from_user_hsv(const user_hsv_color_t * user_hsv);

#endif
