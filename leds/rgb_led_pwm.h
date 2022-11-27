#ifndef __RGB_LED_PWM_H
#define __RGB_LED_PWM_H

#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#include "gpio/c_bsp.h"
#include "colors/colors.h"

#define RGB_LED_PWM_COLOR_COMPONENT_TOP_VALUE 255
#define RGB_LED_PWM_COLOR_COMPONENTS_NUMBER   3

void rgb_led_pwm_enable(bsp_idx_t rgb_led_pwm_idx, nrfx_pwm_t * p_pwm_instance);

void rgb_led_pwm_set_color(bsp_idx_t rgb_led_idx, rgb_color_t rgb);

#endif
