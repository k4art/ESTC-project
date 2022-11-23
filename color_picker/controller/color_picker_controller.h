#ifndef __COLOR_PICKER_CONTROLLER_H
#define __COLOR_PICKER_CONTROLLER_H

#include "color_picker/colors.h"

typedef void (* color_picker_controller_rgb_handler)(rgb_color_t color);

void color_picker_controller_init(void);

void color_picker_controller_set_hsv(hsv_color_t hsv);
void color_picker_controller_enable(uint8_t button_idx);

void color_picker_controller_on_input_change_rgb(color_picker_controller_rgb_handler handler);

#endif
