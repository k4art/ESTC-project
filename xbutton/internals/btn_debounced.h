#ifndef __BTN_DEBOUNCED_H
#define __BTN_DEBOUNCED_H

#include "nrfx.h"

typedef void (*btn_debounced_handler_t)(uint8_t button_idx);


void btn_debounced_init(void);

nrfx_err_t btn_debounced_enable(uint8_t button_idx, bool high_accuracy);

void btn_debounced_on_press(uint8_t button_idx, btn_debounced_handler_t handler);
void btn_debounced_on_release(uint8_t button_idx, btn_debounced_handler_t handler);

bool btn_debounced_is_enabled_for(uint8_t button_idx);

#endif
