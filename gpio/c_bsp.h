#ifndef __C_BSP_H
#define __C_BSP_H

#include <stdint.h>

#include "nrf_gpio.h"

// PCA10059 board

// LEDs
#define LEDS_NUMBER  4

#define LED_1         NRF_GPIO_PIN_MAP(0, 6)
#define LED_2         NRF_GPIO_PIN_MAP(0, 8)
#define LED_3         NRF_GPIO_PIN_MAP(1, 9)
#define LED_4         NRF_GPIO_PIN_MAP(0, 12)

#define LEDS_LIST    { LED_1, LED_2, LED_3, LED_4 }

#define LED_ACTIVE_STATE 0

// Buttons
#define BUTTONS_NUMBER   1

#define BUTTON_1         NRF_GPIO_PIN_MAP(1, 6)

#define BUTTONS_LIST     { BUTTON_1 }

#define BUTTON_PULL           NRF_GPIO_PIN_PULLUP
#define BUTTON_ACTIVE_STATE   0

#define USER_BUTTON_IDX  0

#define IS_VALID_BUTTON_IDX(button_idx) \
  (button_idx >= 0 && button_idx < BUTTONS_NUMBER)

#define IS_VALID_LED_IDX(led_idx) \
  (led_idx >= 0 && led_idx < LEDS_NUMBER)

void c_bsp_board_init(void);
void c_bsp_board_led_invert(uint8_t led_idx);

int c_bsp_board_button_state_get(uint8_t button_idx);

uint8_t c_bsp_board_pin_to_button_idx(uint32_t pin_no);
uint32_t c_bsp_board_button_idx_to_pin(uint8_t button_idx);

#endif

