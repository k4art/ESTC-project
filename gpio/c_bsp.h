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

#define BUTTON_PULL      NRF_GPIO_PIN_PULLUP
#define BUTTON_ACTIVE_STATE 0

#define USER_BUTTON_IDX  0

void c_bsp_board_init(void);
void c_bsp_board_led_invert(size_t led_idx);
int c_bsp_board_button_state_get(size_t button_idx);

#endif

