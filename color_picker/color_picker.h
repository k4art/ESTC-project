#ifndef __COLOR_PICKER_H
#define __COLOR_PICKER_H

#include "gpio/c_bsp.h"
#include "colors/colors.h"

void color_picker_init(void);

void color_picker_set_hsv(hsv_color_t hsv);
void color_picker_restore_or_set_default_hsv(hsv_color_t hsv);

void color_picker_enable(bsp_idx_t button_idx, bsp_idx_t rgb_led_idx, bsp_idx_t status_led_idx);

#endif
