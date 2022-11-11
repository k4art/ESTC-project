#ifndef __BLINKING_H
#define __BLINKING_H

#include "gpio/c_bsp.h"
#include "nrf_delay.h"

#define BLINKS_DELAY_BETWEEN_SERIESES_MS 2000
#define BLINKS_DELAY_BETWEEN_LEDS_MS     1000
#define BLINKS_DELAY_BETWEEN_BLINKS_MS   200
#define BLINKS_BLINK_DURATION_MS         300

struct blinking_iter_info
{
  uint8_t led;
  uint32_t delay_ms;
};

typedef struct blinking_iter_info blinking_series_t[];

size_t repeated_serial_led_blinking(blinking_series_t series,
                                    int blinks_per_leds[LEDS_NUMBER]);

#endif

