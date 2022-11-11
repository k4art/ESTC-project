#include <stdint.h>
#include <stdbool.h>

#include "gpio/c_bsp.h"
#include "blinking.h"

#define DEVICE_ID_RADIX 10

int DEVICE_ID[LEDS_NUMBER] = { 7, 2, 0, 2 };

struct blinking_iter_info series[LEDS_NUMBER * (DEVICE_ID_RADIX - 1)];

void wait_until_button_is_pressed()
{
  while (!c_bsp_board_button_state_get(USER_BUTTON_IDX))
    ;
}

int main(void)
{
  size_t total_iters = repeated_serial_led_blinking(series, DEVICE_ID);

  c_bsp_board_init();

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

