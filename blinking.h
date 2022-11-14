#ifndef __BLINKING_H
#define __BLINKING_H

#include "gpio/c_bsp.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"

#define BLINKS_DELAY_BETWEEN_SERIESES_MS 2000
#define BLINKS_DELAY_BETWEEN_LEDS_MS     1000
#define BLINKS_DELAY_BETWEEN_BLINKS_MS   200
#define BLINKS_BLINK_DURATION_MS         300

#define DUTY_CYCLE_PICK 1000
#define DUTY_CYCLE_ZERO 0

typedef enum
{
  BLINKING_PWM_IN_PROGRESS,
  BLINKING_PWM_REACHED_PICK,
  BLINKING_PWM_ENDED,
} blinking_pwm_status_t;

typedef struct blink_info_s
{
  uint8_t led_idx;
} blink_t;

typedef struct blinking_series_s
{
  blink_t * blinks;
  size_t length;
  size_t capacity;
} blinking_series_t;

#define BLINKING_SERIES_WITH_CAPACITY_DEF(v, cap)   \
  static blink_t v ## _data [(cap)];             \
  static blinking_series_t v = { .capacity = (cap), .blinks = v ## _data } \

typedef struct blinking_pwm_context_s
{
  uint8_t led_idx;
  bool is_time_on;
  int16_t duty_cycle_thousandths_per_cycle;
  uint16_t duty_cycle_thousandths; // 0 to 1000
  nrfx_systick_state_t last_systick_state;
  uint32_t next_waiting_time_us;
} blinking_pwm_context_t;

#define BLINKING_PWM_CONTEXT_START(led_index,                     \
                                   step_thousandths_per_cycle)    \
{                                                                 \
  .led_idx = led_index,                                           \
  .duty_cycle_thousandths_per_cycle = step_thousandths_per_cycle, \
  .duty_cycle_thousandths = DUTY_CYCLE_ZERO,                      \
  .next_waiting_time_us = 0,                                      \
  .is_time_on = 0,                                                \
}

void blinking_repeated_serial_led_init(blinking_series_t * series,
                                       const uint8_t blinks_per_leds[LEDS_NUMBER]);

void blinking_pwm_init(void);

bool blinking_pwm_lighting(blinking_pwm_context_t * context);
blinking_pwm_status_t blinking_pwm_update(blinking_pwm_context_t * context);

#endif

