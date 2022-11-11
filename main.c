#include <stdint.h>
#include <stdbool.h>

#include "blinking.h"

#include "gpio/c_bsp.h"
#include "xbutton/xbutton.h"

#define DEVICE_ID_RADIX 10

int DEVICE_ID[LEDS_NUMBER] = { 7, 2, 0, 2 };

struct blinking_iter_info series[LEDS_NUMBER * (DEVICE_ID_RADIX - 1)];

volatile bool g_button_is_pressed = false;

static void toggle_button_state(uint8_t pin)
{
  (void) pin;

  g_button_is_pressed = !g_button_is_pressed;
}

static void wait_until_button_is_pressed(void)
{
  while (!g_button_is_pressed)
    ;
}

static void initialize(void)
{
  c_bsp_board_init();

  nrfx_err_t err_code;

  err_code = xbutton_init();
  NRFX_ASSERT(err_code == NRFX_SUCCESS);

  err_code = xbutton_enable(USER_BUTTON_IDX, true);
  NRFX_ASSERT(err_code == NRFX_SUCCESS);

  xbutton_on_click(USER_BUTTON_IDX, toggle_button_state);
}

int main(void)
{
  initialize();

  size_t total_iters = repeated_serial_led_blinking(series, DEVICE_ID);

  while (true)
  {
    for (size_t iter_idx = 0; iter_idx < total_iters; iter_idx++)
    {
      struct blinking_iter_info blinking = series[iter_idx];

      wait_until_button_is_pressed();

      c_bsp_board_led_invert(blinking.led);
      nrf_delay_ms(blinking.delay_ms);
    }
  }

  return 0;
}

