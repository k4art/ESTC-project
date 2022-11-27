#include "utils.h"

#include "c_bsp.h"

static const bsp_pin_no_t m_board_leds_list[LEDS_NUMBER] = LEDS_LIST;
static const bsp_pin_no_t m_board_buttons_list[BUTTONS_NUMBER] = BUTTONS_LIST;

static const bsp_rgb_led_t m_board_rgb_led_list[RGB_LEDS_NUMBER] = RGB_LEDS_LIST;

static void c_bsp_board_leds_init(void)
{
  for (size_t i = 0; i < LEDS_NUMBER; ++i)
  {
    nrf_gpio_cfg_output(m_board_leds_list[i]);
    c_bsp_board_led_off(i);
  }
}

static void c_bsp_board_buttons_init(void)
{
  for (size_t i = 0; i < BUTTONS_NUMBER; ++i)
  {
    nrf_gpio_cfg_input(m_board_buttons_list[i], BUTTON_PULL);
  }
}

void c_bsp_board_init(void)
{
  c_bsp_board_leds_init();
  c_bsp_board_buttons_init();
}

void c_bsp_board_led_invert(bsp_idx_t led_idx)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(led_idx));

  nrf_gpio_pin_toggle(m_board_leds_list[led_idx]);
}

void c_bsp_board_led_on(bsp_idx_t led_idx)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(led_idx));

  nrf_gpio_pin_write(m_board_leds_list[led_idx], LED_ACTIVE_STATE);
}

void c_bsp_board_led_off(bsp_idx_t led_idx)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(led_idx));

  nrf_gpio_pin_write(m_board_leds_list[led_idx], LED_ACTIVE_STATE == 1 ? 0 : 1);
}

int c_bsp_board_button_state_get(bsp_idx_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  return nrf_gpio_pin_read(m_board_buttons_list[button_idx]) == BUTTON_ACTIVE_STATE;
}

bsp_idx_t c_bsp_board_pin_to_led_idx(bsp_pin_no_t pin_no)
{
  for (size_t i = 0; i < LEDS_NUMBER; i++)
  {
    if (m_board_leds_list[i] == pin_no)
    {
      return i;
    }
  }

  UNREACHABLE_RETURN(0);
}

bsp_pin_no_t c_bsp_board_led_idx_to_pin(bsp_idx_t led_idx)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(led_idx));

  return m_board_leds_list[led_idx];
}


bsp_idx_t c_bsp_board_pin_to_button_idx(bsp_pin_no_t pin_no)
{
  for (size_t i = 0; i < BUTTONS_NUMBER; i++)
  {
    if (m_board_buttons_list[i] == pin_no)
    {
      return i;
    }
  }

  UNREACHABLE_RETURN(0);
}

bsp_pin_no_t c_bsp_board_button_idx_to_pin(bsp_idx_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  return m_board_buttons_list[button_idx];
}

const bsp_rgb_led_t * c_bsp_board_rgb_led_idx_to_pins(bsp_idx_t rgb_led_idx)
{
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));

  return &m_board_rgb_led_list[rgb_led_idx];
}
