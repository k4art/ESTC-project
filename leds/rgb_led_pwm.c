#include "nrf_log.h"
#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#include "gpio/c_bsp.h"

#include "rgb_led_pwm.h"

#define RGB_LED_PWM_TOP_VALUE RGB_LED_PWM_COLOR_COMPONENT_TOP_VALUE
#define RGB_LED_PWM_CLK       NRF_PWM_CLK_125kHz
#define RGB_LED_IRQ_PRIORITY  NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY

#define P_RGB_LED_PWM_CHANNEL_OUTPUT_PINS(p_rgb_led) \
  {                                                  \
    p_rgb_led->red_pin,                              \
    p_rgb_led->green_pin,                            \
    p_rgb_led->blue_pin,                             \
    NRFX_PWM_PIN_NOT_USED,                           \
  }

typedef struct rgb_led_info_s
{
  bool is_used;
  nrfx_pwm_t * p_pwm_inst;
  nrf_pwm_sequence_t pwm_seq;
} rgb_led_pwm_info_t;

typedef struct
{
  rgb_led_pwm_info_t rgb_leds[RGB_LEDS_NUMBER];
} rgb_led_pwm_control_block_t;

static rgb_led_pwm_control_block_t m_cb;

static bool rgb_led_is_used(uint8_t rgb_led_idx)
{
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));
  return m_cb.rgb_leds[rgb_led_idx].is_used;
}

static nrfx_err_t pwm_led_pwm_config_pwm_instance(uint8_t rgb_led_idx)
{
  const rgb_led_t * p_rgb_led = c_bsp_board_rgb_led_idx_to_pins(rgb_led_idx);

  nrfx_pwm_config_t config =
  {
    .output_pins = P_RGB_LED_PWM_CHANNEL_OUTPUT_PINS(p_rgb_led),
    .irq_priority = RGB_LED_IRQ_PRIORITY,
    .base_clock = RGB_LED_PWM_CLK,
    .top_value = RGB_LED_PWM_TOP_VALUE,
    .count_mode = NRF_PWM_MODE_UP,
    .load_mode = NRF_PWM_LOAD_INDIVIDUAL,
    .step_mode = NRF_PWM_STEP_AUTO,
  };

  return nrfx_pwm_init(m_cb.rgb_leds[rgb_led_idx].p_pwm_inst, &config, NULL);
}

nrfx_err_t rgb_led_pwm_enable(uint8_t rgb_led_idx, nrfx_pwm_t * p_pwm_instance, rgb_led_pwm_color_t * p_color)
{
  NRFX_ASSERT(!rgb_led_is_used(rgb_led_idx));

  m_cb.rgb_leds[rgb_led_idx].p_pwm_inst = p_pwm_instance;

  m_cb.rgb_leds[rgb_led_idx].pwm_seq = (nrf_pwm_sequence_t)
  {
    .values.p_raw = (uint16_t *) p_color->components,
    .length = NRF_PWM_VALUES_LENGTH(*p_color),
  };

  return pwm_led_pwm_config_pwm_instance(rgb_led_idx);
}

void rgb_led_pwm_turn_on(uint8_t rgb_led_idx)
{
  NRFX_ASSERT(rgb_led_is_used(rgb_led_idx));

  nrfx_pwm_simple_playback(m_cb.rgb_leds[rgb_led_idx].p_pwm_inst,
                           &m_cb.rgb_leds[rgb_led_idx].pwm_seq,
                           1,
                           NRFX_PWM_FLAG_LOOP);
}
