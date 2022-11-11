#include "c_bsp.h"

static const uint8_t m_board_led_list[LEDS_NUMBER] = LEDS_LIST;
static const uint8_t m_board_buttons_list[BUTTONS_NUMBER] = BUTTONS_LIST;

static void c_bsp_board_leds_init(void)
{
  for (uint32_t i = 0; i < LEDS_NUMBER; ++i)
  {
    nrf_gpio_cfg_output(m_board_led_list[i]);
    nrf_gpio_pin_write(m_board_led_list[i], !LED_ACTIVE_STATE);
  }
}

static void c_bsp_board_buttons_init(void)
{
  for (uint32_t i = 0; i < BUTTONS_NUMBER; ++i)
  {
    nrf_gpio_cfg_input(m_board_buttons_list[i], BUTTON_PULL);
  }
}

void c_bsp_board_init(void)
{
  c_bsp_board_leds_init();
  c_bsp_board_buttons_init();
}

void c_bsp_board_led_invert(uint8_t led_idx)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(led_idx));

  nrf_gpio_pin_toggle(m_board_led_list[led_idx]);
}

int c_bsp_board_button_state_get(uint8_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  return nrf_gpio_pin_read(m_board_buttons_list[button_idx]) == BUTTON_ACTIVE_STATE;
}

uint8_t c_bsp_board_pin_to_button_idx(uint32_t pin_no)
{
  for (size_t i = 0; i < BUTTONS_NUMBER; i++)
  {
    if (m_board_buttons_list[i] == pin_no)
    {
      return i;
    }
  }

  NRFX_ASSERT(0);
  return 0; // should not be reached
}

uint32_t c_bsp_board_button_idx_to_pin(uint8_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  return m_board_buttons_list[button_idx];
}

