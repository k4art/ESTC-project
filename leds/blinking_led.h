#ifndef __BLINKING_H
#define __BLINKING_H

#include "nrfx_pwm.h"

#include "gpio/c_bsp.h"

typedef enum blinking_mode_e
{
  BLINKING_LED_MODE_OFF,
  BLINKING_LED_MODE_ON,
  BLINKING_LED_MODE_BLINKS_SLOW,
  BLINKING_LED_MODE_BLINKS_FAST,
} blinking_led_mode_t;

typedef struct blinky_led_s
{
  bsp_idx_t led_idx;

  /* In constract to singular intensity. */
  bool is_blinking;

  nrfx_pwm_t * p_pwm_inst;
  nrf_pwm_sequence_t pwm_seq;
} blinking_led_t;

void blinking_led_enable(blinking_led_t * led, bsp_idx_t led_idx, nrfx_pwm_t * p_pwm_instanced);
void blinking_led_set_mode(blinking_led_t * led, blinking_led_mode_t mode);

#endif
