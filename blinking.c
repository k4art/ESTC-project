#include "nrf_delay.h"
#include "nrf_log.h"

#include "gpio/c_bsp.h"

#include "blinking.h"

#define PWM_FREQUENCE_HZ 1000

const uint32_t PWM_PERIOD_US = 1000000 / PWM_FREQUENCE_HZ;

void blinking_repeated_serial_led_init(blinking_series_t * series,
                                       const uint8_t blinks_per_leds[LEDS_NUMBER])
{
  size_t blink_idx = 0;

  for (size_t led_idx = 0; led_idx < LEDS_NUMBER; led_idx++)
  {
    for (uint8_t i = 0; i < blinks_per_leds[led_idx]; i++)
    {
      series->blinks[blink_idx++] = (blink_t) { .led_idx = led_idx };
    }
  }

  series->length = blink_idx;
}

static bool pwm_reached_pick(blinking_pwm_context_t * context)
{
  return context->duty_cycle_thousandths >= DUTY_CYCLE_PICK
    && context->duty_cycle_thousandths_per_cycle > 0;
}

static bool pwm_reached_end(blinking_pwm_context_t * context)
{
  return context->duty_cycle_thousandths <= DUTY_CYCLE_ZERO
    && context->duty_cycle_thousandths_per_cycle < 0;
}

static uint32_t pwm_calc_time_on_us(blinking_pwm_context_t * context)
{
  return PWM_PERIOD_US * context->duty_cycle_thousandths / 1000;
}

void blinking_pwm_init(void)
{
  nrfx_systick_init();
}

bool blinking_pwm_lighting(blinking_pwm_context_t * context)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(context->led_idx));

  if (nrfx_systick_test(&context->last_systick_state, context->waiting_time_us))
  {
    bool should_invert_led = true;
    nrfx_systick_get(&context->last_systick_state);

    context->is_time_on = !context->is_time_on;

    if (context->is_time_on)
    {
      context->waiting_time_us = pwm_calc_time_on_us(context);
      should_invert_led = context->waiting_time_us != 0;
    }
    else
    {
      context->waiting_time_us = PWM_PERIOD_US - pwm_calc_time_on_us(context);
      should_invert_led = context->waiting_time_us != PWM_PERIOD_US;
    }

    // this logic allows correct handle of extreme cases when duty cycle is 0% or 100%
    if (should_invert_led)
    {
      c_bsp_board_led_invert(context->led_idx);
    }

    return !context->is_time_on;
  }

  return false;
}

blinking_pwm_status_t blinking_pwm_update(blinking_pwm_context_t * context)
{
  NRFX_ASSERT(IS_VALID_LED_IDX(context->led_idx));

  NRFX_ASSERT(!pwm_reached_pick(context));
  NRFX_ASSERT(!pwm_reached_end(context));

  NRFX_ASSERT(context->duty_cycle_thousandths_per_cycle != 0);

  context->duty_cycle_thousandths += context->duty_cycle_thousandths_per_cycle;

  if (pwm_reached_pick(context))
  {
    NRF_LOG_INFO("[blinking]: (%d) pwm reached pick.", context->led_idx);

    context->duty_cycle_thousandths_per_cycle = -context->duty_cycle_thousandths_per_cycle;
    context->duty_cycle_thousandths = DUTY_CYCLE_PICK;

    return BLINKING_PWM_REACHED_PICK;
  }

  if (pwm_reached_end(context)) {
    NRF_LOG_INFO("[blinking]: (%d) pwm reached end.", context->led_idx);

    context->duty_cycle_thousandths = DUTY_CYCLE_ZERO;

    return BLINKING_PWM_ENDED;
  }

  return BLINKING_PWM_IN_PROGRESS;
}
