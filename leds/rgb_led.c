#include "nrf_log.h"
#include "nrf_pwm.h"
#include "nrfx_pwm.h"

#include "rgb_led.h"

#define RGB_LED_PWM_TOP_VALUE RGB_COMPONENTS_TOP_VALUE

#define RGB_LED_PWM_CLK       NRF_PWM_CLK_125kHz
#define RGB_LED_IRQ_PRIORITY  NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY

// The fields must be uint16_t, because PWM works only with 16 bits values sequences
typedef struct rgb_led_color_s
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
} rgb_led_color_t;

STATIC_ASSERT(NRF_PWM_VALUES_LENGTH(rgb_led_color_t) == NRF_PWM_CHANNEL_COUNT);

#define RGB_LED_CHANNEL_OUTPUT_PINS(p_rgb_led) \
  (rgb_led_t)                                  \
  {                                            \
    p_rgb_led->red_pin,                        \
    p_rgb_led->green_pin,                      \
    p_rgb_led->blue_pin,                       \
    NRFX_PWM_PIN_NOT_USED,                     \
  }

typedef struct rgb_led_info_s
{
  bool is_used;
  nrf_pwm_sequence_t pwm_seq;
  nrfx_pwm_t * p_pwm_inst;
  rgb_led_color_t p_pwm_color;
} rgb_led_info_t;

typedef struct
{
  rgb_led_info_t rgb_leds[RGB_LEDS_NUMBER];
} rgb_led_control_block_t;

static rgb_led_control_block_t m_cb;

static bool rgb_led_is_used(bsp_idx_t rgb_led_idx)
{
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));
  return m_cb.rgb_leds[rgb_led_idx].is_used;
}

static nrfx_err_t rgb_led_config_pwm_instance(bsp_idx_t rgb_led_idx)
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

  return nrfx_pwm_init(m_cb.rgb_leds[rgb_led_idx].p_pwm_inst, &config, NULL);
}

void rgb_led_enable(bsp_idx_t rgb_led_idx, nrfx_pwm_t * p_pwm_instance)
{
  NRFX_ASSERT(!rgb_led_is_used(rgb_led_idx));

  m_cb.rgb_leds[rgb_led_idx].p_pwm_inst = p_pwm_instance;

  m_cb.rgb_leds[rgb_led_idx].pwm_seq = (nrf_pwm_sequence_t)
  {
    .values.p_raw = m_cb.rgb_leds[rgb_led_idx].p_pwm_color.pwm_individual_values,
    .length       = 4,
  };

  nrfx_err_t err_code = rgb_led_config_pwm_instance(rgb_led_idx);
  APP_ERROR_CHECK(err_code);

  nrfx_pwm_simple_playback(m_cb.rgb_leds[rgb_led_idx].p_pwm_inst,
                           &m_cb.rgb_leds[rgb_led_idx].pwm_seq,
                           1,
                           NRFX_PWM_FLAG_LOOP);
}

void rgb_led_set_color(bsp_idx_t rgb_led_idx, rgb_color_t rgb)
{
  NRF_LOG_INFO("[rgb_led]: set color %d:%d:%d", rgb.red, rgb.green, rgb.blue);

  m_cb.rgb_leds[rgb_led_idx].p_pwm_color.red = rgb.red;
  m_cb.rgb_leds[rgb_led_idx].p_pwm_color.green = rgb.green;
  m_cb.rgb_leds[rgb_led_idx].p_pwm_color.blue = rgb.blue;
}
