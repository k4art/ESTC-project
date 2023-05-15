#ifndef RGB_LED_PWM_H__
#define RGB_LED_PWM_H__

#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#include "lib/gpio/c_bsp.h"
#include "domain/colors/colors.h"

typedef struct rgb_led_pwm_values_s
{
  union
  {
    struct
    {
      uint16_t red;
      uint16_t green;
      uint16_t blue;
    };

    uint16_t pwm_individual_values[NRF_PWM_CHANNEL_COUNT];
  };
} rgb_led_pwm_values_t;

STATIC_ASSERT(NRF_PWM_VALUES_LENGTH(rgb_led_pwm_values_t) == NRF_PWM_CHANNEL_COUNT);

#define RGB_LED_PWM_VALUES(r, g, b)      \
  (rgb_led_pwm_values_t)                 \
  {                                      \
    .pwm_individual_values = { r, g, b } \
  }

typedef struct rgb_led_s
{
  bsp_idx_t rgb_led_idx;
  rgb_led_pwm_values_t pwm_values;
  nrf_pwm_sequence_t pwm_seq;
  nrfx_pwm_t * p_pwm_inst;
} rgb_led_t;

void rgb_led_enable(rgb_led_t * rgb_led, bsp_idx_t rgb_led_idx, nrfx_pwm_t * p_pwm_instance);
void rgb_led_set_color(rgb_led_t * rgb_led, rgb_color_t rgb);

#endif
