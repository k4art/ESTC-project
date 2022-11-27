#ifndef __C_BSP_H
#define __C_BSP_H

#include <stdint.h>

#include "nrf_gpio.h"

typedef uint8_t bsp_pin_no_t;
typedef uint8_t bsp_idx_t;

// PCA10059 board

// LEDs
#define LEDS_NUMBER  4

#define LED_1            NRF_GPIO_PIN_MAP(0, 6)
#define LED_2            NRF_GPIO_PIN_MAP(0, 8)
#define LED_3            NRF_GPIO_PIN_MAP(1, 9)
#define LED_4            NRF_GPIO_PIN_MAP(0, 12)

#define LED2_R           LED_2
#define LED2_G           LED_3
#define LED2_B           LED_4

#define LEDS_LIST        { LED_1, LED_2, LED_3, LED_4 }

#define LED_ACTIVE_STATE 0

// Buttons
#define BUTTONS_NUMBER          1

#define BUTTON_1                NRF_GPIO_PIN_MAP(1, 6)

#define BUTTONS_LIST            { BUTTON_1 }

#define BUTTON_PULL             NRF_GPIO_PIN_PULLUP
#define BUTTON_ACTIVE_STATE     0
#define BUTTON_BOUNCING_TIME_MS 50

#define USER_BUTTON_IDX         0

// RGB LEDs
typedef struct
{
  bsp_pin_no_t red_pin;
  bsp_pin_no_t green_pin;
  bsp_pin_no_t blue_pin;
} rgb_led_t;

#define RGB_LEDS_NUMBER  1

#define RGB_LED_1        { .red_pin = LED2_R, .green_pin = LED2_G, .blue_pin = LED2_B }

#define RGB_LEDS_LIST    { RGB_LED_1 }

#define ONLY_RGB_LED_IDX 0

#define IS_VALID_BUTTON_IDX(button_idx) \
  (button_idx >= 0 && button_idx < BUTTONS_NUMBER)

#define IS_VALID_LED_IDX(led_idx) \
  (led_idx >= 0 && led_idx < LEDS_NUMBER)

#define IS_VALID_RGB_LED_IDX(rgb_led_idx) \
  (rgb_led_idx >= 0 && rgb_led_idx < RGB_LEDS_NUMBER)

void c_bsp_board_init(void);

void c_bsp_board_led_invert(bsp_idx_t led_idx);
void c_bsp_board_led_on(bsp_idx_t led_idx);
void c_bsp_board_led_off(bsp_idx_t led_idx);

int c_bsp_board_button_state_get(bsp_idx_t button_idx);


bsp_idx_t c_bsp_board_pin_to_button_idx(bsp_pin_no_t pin_no);
bsp_pin_no_t c_bsp_board_button_idx_to_pin(bsp_idx_t button_idx);

const rgb_led_t * c_bsp_board_rgb_led_idx_to_pins(bsp_idx_t rgb_led_idx);

#endif
