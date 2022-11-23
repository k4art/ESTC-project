#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "gpio/c_bsp.h"
#include "color_picker/color_picker.h"

#define DEVICE_ID 7202

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
  color_picker_init();
}

static hsv_color_t calc_default_hsv(uint16_t device_id)
{
  uint16_t hue_perc = device_id % 100;

  uint8_t h = hue_perc * H_COMPONENT_TOP_VALUE / 100;
  uint8_t s = S_COMPONENT_TOP_VALUE;
  uint8_t v = V_COMPONENT_TOP_VALUE;

  return (hsv_color_t) HSV_COLOR(h, s, v);
}

int main(void)
{
  initialize();

  hsv_color_t default_color = calc_default_hsv(DEVICE_ID);

  color_picker_set_hsv(default_color);
  color_picker_enable(USER_BUTTON_IDX, ONLY_RGB_LED_IDX);

  while (true)
  {
    keep_usb_connection();
  }

  return 0;
}
