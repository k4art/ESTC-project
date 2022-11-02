#ifndef __BLINKING_H
#define __BLINKING_H

#include "nrf_delay.h"

#define BLINKS_DELAY_BETWEEN_SERIESES 2000
#define BLINKS_DELAY_BETWEEN_LEDS 1000
#define BLINKS_DELAY_BETWEEN_BLINKS 200
#define BLINKS_BLINK_DURATION 300

struct blinking_iter_info
{
  char led;
  uint32_t delay;
};

typedef struct blinking_iter_info blinking_series[];

size_t repeated_serial_led_blinking(blinking_series series,
                                    int blinks_per_leds[LEDS_NUMBER]);

#endif

