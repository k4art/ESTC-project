#ifndef __COLOR_PICKER_H
#define __COLOR_PICKER_H

#include "color_picker/colors.h"

void color_picker_init(void);

void color_picker_set_hsv(hsv_color_t hsv);
void color_picker_enable(uint8_t button_idx, uint8_t rgb_led_idx);

#endif
