#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"

#include "ble/stack/init.h"
#include "cli/cli.h"
#include "lib/gpio/c_bsp.h"
#include "lib/utils.h"
#include "io/input.h"
#include "domain/color_picker/color_picker.h"
#include "domain/events/events.h"

#define DEVICE_ID 7202

static bool keep_usb_logs_alive(void)
{
  LOG_BACKEND_USB_PROCESS();
  return NRF_LOG_PROCESS();
}

static void logs_init(void)
{
  CHECKED(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void initialize(void)
{
  logs_init();
  c_bsp_board_init();

  CHECKED(app_timer_init());
  CHECKED(nrf_pwr_mgmt_init());
  ble_app_startup();

#if NRFX_CHECK(ESTC_USB_CLI_ENABLED)
  cli_init();
#endif

  color_picker_init();
}

static hsv_color_t calc_default_hsv(uint16_t device_id)
{
  uint16_t hue_perc = device_id % 100;

  uint8_t h = NRFX_ROUNDED_DIV(hue_perc * H_COMPONENT_TOP_VALUE, 100);
  uint8_t s = S_COMPONENT_TOP_VALUE;
  uint8_t v = V_COMPONENT_TOP_VALUE;

  return HSV_COLOR(h, s, v);
}

int main(void)
{
  initialize();

  hsv_color_t default_color = calc_default_hsv(DEVICE_ID);

  color_picker_restore_or_set_default_hsv(default_color);
  color_picker_enable(USER_BUTTON_IDX, ONLY_RGB_LED_IDX, SINGLE_LED_IDX);

  while (true)
  {
#if NRFX_CHECK(ESTC_USB_CLI_ENABLED)
    cli_process();
#endif

    app_events_process();
    app_input_process();

    if (!keep_usb_logs_alive()) nrf_pwr_mgmt_run();
  }

  return 0;
}
