#ifndef __BLINKING_H
#define __BLINKING_H

#include "nrfx_pwm.h"

#include "gpio/c_bsp.h"

typedef enum blinking_mode_e
{
  BLINKING_MODE_OFF,
  BLINKING_MODE_ON,
  BLINKING_MODE_BLINKS_SLOW,
  BLINKING_MODE_BLINKS_FAST,
} blinking_mode_t;

typedef struct blinky_led_s
{
  bsp_idx_t led_idx;

  /* In constract to singular intensity. */
  bool is_blinking;

  nrfx_pwm_t * p_pwm_inst;
  nrf_pwm_sequence_t pwm_seq;
} blinking_led_t;

#define BLINKING_LED(led_index, p_pwm_instance) \
  (blinking_led_t)                              \
  {                                             \
    .led_idx = led_index,                       \
    .p_pwm_inst = p_pwm_instance,               \
  }

void blinking_enable(blinking_led_t * led);
void blinking_set_mode(blinking_led_t * led, blinking_mode_t mode);

#endif
