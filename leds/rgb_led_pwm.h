#ifndef __RGB_LED_PWM_H
#define __RGB_LED_PWM_H

#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#define RGB_LED_PWM_COLOR_COMPONENT_TOP_VALUE 255
#define RGB_LED_PWM_COLOR_COMPONENTS_NUMBER   3

// The fields must be uint16_t, because PWM works only with 16 bits values sequences
typedef struct rgb_led_pwm_color_s
{
  union
  {
    struct __ALIGNED(2)
    {
      uint16_t red;
      uint16_t green;
      uint16_t blue;
    };

    uint16_t components[RGB_LED_PWM_COLOR_COMPONENTS_NUMBER];

    // PWM sequences should be multiples of NRF_PWM_CHANNEL_COUNT * uint16_t in size
    uint16_t _fix_size[NRF_PWM_CHANNEL_COUNT];
  };
} rgb_led_pwm_color_t;

STATIC_ASSERT(NRF_PWM_VALUES_LENGTH(rgb_led_pwm_color_t) == 4);

nrfx_err_t rgb_led_pwm_enable(uint8_t rgb_led_pwm_idx,
                              nrfx_pwm_t * p_pwm_instance,
                              rgb_led_pwm_color_t * p_color);

void rgb_led_pwm_turn_on(uint8_t rgb_led_idx);

const uint16_t * rgb_led_pwm_get_raw_seq(uint8_t rgb_led_pwm_idx);

// void rgb_led_pwm_color_change(rgb_led_pwm_color_t * rgb_pwm_color, rgb_color_t rgb_color);

#endif
