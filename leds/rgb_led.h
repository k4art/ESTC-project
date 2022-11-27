#ifndef __RGB_LED_PWM_H
#define __RGB_LED_PWM_H

#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#include "gpio/c_bsp.h"
#include "colors/colors.h"

void rgb_led_enable(bsp_idx_t rgb_led_pwm_idx, nrfx_pwm_t * p_pwm_instance);

void rgb_led_set_color(bsp_idx_t rgb_led_idx, rgb_color_t rgb);

#endif
