
#include "c_bsp.h"

static const uint8_t c_board_led_list[LEDS_NUMBER] = LEDS_LIST;
static const uint8_t c_board_buttons_list[BUTTONS_NUMBER] = BUTTONS_LIST;

static void c_bsp_board_leds_init(void)
{
  for (uint32_t i = 0; i < LEDS_NUMBER; ++i)
  {
    nrf_gpio_cfg_output(c_board_led_list[i]);
    nrf_gpio_pin_write(c_board_led_list[i], !LED_ACTIVE_STATE);
  }
}

static void c_bsp_board_buttons_init(void)
{
  for (uint32_t i = 0; i < BUTTONS_NUMBER; ++i)
  {
    nrf_gpio_cfg_input(c_board_buttons_list[i], BUTTON_PULL);
  }
}

void c_bsp_board_init(void)
{
  c_bsp_board_leds_init();
  c_bsp_board_buttons_init();
}

void c_bsp_board_led_invert(size_t led_idx)
{
  nrf_gpio_pin_toggle(c_board_led_list[led_idx]);
}

int c_bsp_board_button_state_get(size_t button_idx)
{
  return nrf_gpio_pin_read(c_board_buttons_list[button_idx]) == BUTTON_ACTIVE_STATE;
}

