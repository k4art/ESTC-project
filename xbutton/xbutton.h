#ifndef __XBUTTON_H
#define __XBUTTON_H

#include <stdbool.h>
#include <stdint.h>

#include <nrfx.h>

typedef void (*xbutton_handler_t)(uint8_t button_idx);

void xbutton_init(void);

nrfx_err_t xbutton_enable(uint8_t button_idx, bool high_accuracy);

void xbutton_on_click(uint8_t button_idx, xbutton_handler_t handler);
void xbutton_on_double_click(uint8_t button_idx, xbutton_handler_t handler);

void xbutton_on_long_press_stop(uint8_t button_idx, xbutton_handler_t handler);
void xbutton_on_long_press_start(uint8_t button_idx, xbutton_handler_t handler);

#endif
