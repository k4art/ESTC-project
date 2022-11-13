#ifndef __BTN_CLICKABLE_H
#define __BTN_CLICKABLE_H

#include <stdbool.h>
#include <stdint.h>

#include <nrfx.h>

typedef void (*btn_clickable_handler_t)(uint8_t button_idx);

void btn_clickable_init(void);

nrfx_err_t btn_clickable_enable(uint8_t button_idx, bool high_accuracy);

void btn_clickable_on_press(uint8_t button_idx, btn_clickable_handler_t handler);
void btn_clickable_on_release(uint8_t button_idx, btn_clickable_handler_t handler);

void btn_clickable_on_click(uint8_t button_idx, btn_clickable_handler_t handler);

#endif
