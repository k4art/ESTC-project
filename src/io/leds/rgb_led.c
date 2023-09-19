#include "nrf_log.h"
#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#include "io/leds/rgb_led.h"

#define RGB_LED_PWM_TOP_VALUE RGB_COMPONENTS_TOP_VALUE

#define RGB_LED_PWM_CLK       NRF_PWM_CLK_125kHz
#define RGB_LED_IRQ_PRIORITY  NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY

#define RGB_LED_CHANNEL_OUTPUT_PINS(p_rgb_led) \
  {                                            \
    p_rgb_led->red_pin,                        \
    p_rgb_led->green_pin,                      \
    p_rgb_led->blue_pin,                       \
    NRFX_PWM_PIN_NOT_USED,                     \
  }

static nrfx_err_t rgb_led_config_pwm_instance(rgb_led_t * rgb_led)
{
  const bsp_rgb_led_t * p_bsp_rgb_led = c_bsp_board_rgb_led_idx_to_pins(rgb_led->rgb_led_idx);

  nrfx_pwm_config_t config =
  {
    .output_pins  = RGB_LED_CHANNEL_OUTPUT_PINS(p_bsp_rgb_led),
    .irq_priority = RGB_LED_IRQ_PRIORITY,
    .base_clock   = RGB_LED_PWM_CLK,
    .top_value    = RGB_LED_PWM_TOP_VALUE,
    .count_mode   = NRF_PWM_MODE_UP,
    .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
    .step_mode    = NRF_PWM_STEP_AUTO,
  };

  return nrfx_pwm_init(rgb_led->p_pwm_inst, &config, NULL);
}


void rgb_led_enable(rgb_led_t * rgb_led, bsp_idx_t rgb_led_idx, nrfx_pwm_t * p_pwm_instance)
{
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));

  rgb_led->rgb_led_idx = rgb_led_idx;
  rgb_led->p_pwm_inst  = p_pwm_instance;

  rgb_led->pwm_seq = (nrf_pwm_sequence_t)
  {
    .values.p_raw = rgb_led->pwm_values.pwm_individual_values,
    .length       = 4,
  };

  nrfx_err_t err_code = rgb_led_config_pwm_instance(rgb_led);
  APP_ERROR_CHECK(err_code);

  nrfx_pwm_simple_playback(rgb_led->p_pwm_inst,
                           &rgb_led->pwm_seq,
                           1,
                           NRFX_PWM_FLAG_LOOP);
}

void rgb_led_set_color(rgb_led_t * rgb_led, rgb_color_t rgb)
{
  NRF_LOG_INFO("[rgb_led]: set color %d:%d:%d", rgb.red, rgb.green, rgb.blue);

  rgb_led->pwm_values.red   = rgb.red;
  rgb_led->pwm_values.green = rgb.green;
  rgb_led->pwm_values.blue  = rgb.blue;
}
