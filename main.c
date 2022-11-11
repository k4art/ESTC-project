#include <stdint.h>
#include <stdbool.h>

#include "blinking.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "gpio/c_bsp.h"
#include "xbutton/xbutton.h"

#define DEVICE_ID_RADIX 10

int DEVICE_ID[LEDS_NUMBER] = { 7, 2, 0, 2 };

struct blinking_iter_info series[LEDS_NUMBER * (DEVICE_ID_RADIX - 1)];

volatile bool g_blinking_should_proceed = false;

static void toggle_should_proceed_flag(uint8_t pin)
{
  (void) pin;

  g_blinking_should_proceed = !g_blinking_should_proceed;

  NRF_LOG_INFO("Switched! Blinking is going: %d", g_blinking_should_proceed);
}

static void keep_usb_connection(void)
{
  LOG_BACKEND_USB_PROCESS();
  NRF_LOG_PROCESS();
}

static void wait_ms_but_keep_usb(uint32_t delay_ms)
{
  const uint32_t SLEEP_DURATION_MS = 1;

  uint32_t left_to_wait_ms = delay_ms;

  while (left_to_wait_ms > 0)
  {
    keep_usb_connection();

    nrf_delay_ms(SLEEP_DURATION_MS);
    left_to_wait_ms -= SLEEP_DURATION_MS;
  }
}

static void wait_until_blicking_should_proceed(void)
{
  while (!g_blinking_should_proceed)
  {
    keep_usb_connection();
  }
}

static void logs_init(void)
{
  ret_code_t ret = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(ret);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void initialize(void)
{
  logs_init();

  c_bsp_board_init();

  nrfx_err_t err_code;

  err_code = xbutton_init();
  NRFX_ASSERT(err_code == NRFX_SUCCESS);

  err_code = xbutton_enable(USER_BUTTON_IDX, true);
  NRFX_ASSERT(err_code == NRFX_SUCCESS);

  xbutton_on_click(USER_BUTTON_IDX, toggle_should_proceed_flag);
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

      wait_until_blicking_should_proceed();

      c_bsp_board_led_invert(blinking.led);
      wait_ms_but_keep_usb(blinking.delay_ms);
    }
  }

  return 0;
}

