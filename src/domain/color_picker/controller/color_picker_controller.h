#ifndef COLOR_PICKER_CONTROLLER_H__
#define COLOR_PICKER_CONTROLLER_H__

#include "domain/colors/colors.h"
#include "io/leds/blinking_led.h"

typedef void (* color_picker_controller_hsv_handler_t)(hsv_color_t color);

void color_picker_controller_init(void);

void color_picker_controller_set_hsv(hsv_color_t hsv);
hsv_color_t color_picker_controller_get_hsv(void);

/*
 * Note: p_status_led must be enabled.
 */
void color_picker_controller_enable(bsp_idx_t button_idx, blinking_led_t * p_status_led);

void color_picker_controller_on_input_change_hsv(color_picker_controller_hsv_handler_t handler);

void color_picker_controller_on_edit_end(color_picker_controller_hsv_handler_t handler);

#endif
