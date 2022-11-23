#ifndef __COLOR_PICKER_CONTROLLER_H
#define __COLOR_PICKER_CONTROLLER_H

#include "color_picker/colors.h"

typedef void (* color_picker_controller_hsv_handler_fn)(hsv_color_t color);

void color_picker_controller_init(void);

void color_picker_controller_set_hsv(hsv_color_t hsv);
hsv_color_t color_picker_controller_get_hsv(void);

void color_picker_controller_enable(uint8_t button_idx);

void color_picker_controller_on_input_change_hsv(color_picker_controller_hsv_handler_fn handler);

#endif
