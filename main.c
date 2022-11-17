#include <stdint.h>
#include <stdbool.h>

#include "blinking.h"
#include "nrfx_systick.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "gpio/c_bsp.h"
#include "xbutton/xbutton.h"

#define DEVICE_ID_RADIX 10

uint8_t DEVICE_ID[LEDS_NUMBER] = { 7, 2, 0, 2 };

BLINKING_SERIES_WITH_CAPACITY_DEF(series, LEDS_NUMBER * (DEVICE_ID_RADIX - 1));

volatile bool g_pwm_freeze = true;

static void toggle_should_proceed_flag(uint8_t pin)
{
  (void) pin;

  g_pwm_freeze = !g_pwm_freeze;
}

static void keep_usb_connection(void)
{
  LOG_BACKEND_USB_PROCESS();
  NRF_LOG_PROCESS();
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
  xbutton_init();
  blinking_pwm_init();

  nrfx_err_t err_code = xbutton_enable(USER_BUTTON_IDX, true);
  NRFX_ASSERT(err_code == NRFX_SUCCESS);

  xbutton_on_double_click(USER_BUTTON_IDX, toggle_should_proceed_flag);
}

int main(void)
{
  initialize();

  blinking_repeated_serial_led_init(&series, DEVICE_ID);

  while (true)
  {
    for (size_t blink_idx = 0; blink_idx < series.length; blink_idx++)
    {
      blink_t blink = series.blinks[blink_idx];

      blinking_pwm_context_t ctx = BLINKING_PWM_CONTEXT_START(blink.led_idx, 2);
      blinking_pwm_status_t pwm_status = BLINKING_PWM_IN_PROGRESS;

      do {
        bool is_time_to_update = blinking_pwm_lighting(&ctx);

        if (!g_pwm_freeze && is_time_to_update)
        {
          pwm_status = blinking_pwm_update(&ctx);
        }

        keep_usb_connection();
      } while (pwm_status != BLINKING_PWM_ENDED);

      NRF_LOG_INFO("[main]: moving to the next LED");
    }
  }

  return 0;
}

