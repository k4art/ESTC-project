#include "nrf_delay.h"

#include "gpio/c_bsp.h"

#include "blinking.h"

size_t repeated_serial_led_blinking(blinking_series_t series,
                                    int blinks_per_leds[LEDS_NUMBER])
{
  size_t iter_idx = 0;

  for (size_t led_id = 0; led_id < LEDS_NUMBER; led_id++)
  {
    size_t blinks = blinks_per_leds[led_id];

    if (blinks == 0 && iter_idx > 0)
    {
      series[iter_idx - 1].delay += BLINKS_DELAY_BETWEEN_LEDS;
      continue;
    }

    for (size_t b = 0; b < blinks; b++)
    {
      series[iter_idx].led = led_id;
      series[iter_idx++].delay = BLINKS_BLINK_DURATION;

      series[iter_idx].led = led_id;
      series[iter_idx++].delay = BLINKS_DELAY_BETWEEN_BLINKS;
    }

    if (iter_idx > 0)
    {
      series[iter_idx - 1].delay = BLINKS_DELAY_BETWEEN_LEDS;
    }
  }

  series[iter_idx - 1].delay = BLINKS_DELAY_BETWEEN_SERIESES;

  return iter_idx;
}
