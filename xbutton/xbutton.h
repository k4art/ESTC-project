#ifndef __XBUTTON_H
#define __XBUTTON_H

#include <stdbool.h>
#include <stdint.h>

#include <nrfx.h>

typedef void (*xbutton_handler_t)(uint8_t pin);

nrfx_err_t xbutton_init(void);

nrfx_err_t xbutton_enable(uint8_t button_pin, bool high_accuracy);

void xbutton_on_click(uint8_t button_pin, xbutton_handler_t handler);

#endif
