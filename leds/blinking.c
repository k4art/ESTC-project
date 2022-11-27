#include "nrf_log.h"
#include "nrfx_pwm.h"

#include "gpio/c_bsp.h"

#include "leds/blinking.h"

// should be specified in both macros
#define BLINKING_PWM_CONFIG_BASE_CLOCK   NRF_PWM_CLK_125kHz
#define BLINKING_PWM_CLOCK_FREQUENCY_KHZ 125

#define BLINKING_MODE_SLOW_BLINK_PERIOD_MS 1500
#define BLINKING_MODE_FAST_BLINK_PERIOD_MS 500

#define BLINKING_SERIES_LENGTH 128

// 1250 top value implies 100 HZ PWM frequency with slowest (125kHZ) base clock
const uint16_t TOP_VALUE_FOR_SINGLULAR_INTENSITY = 1250;

static nrf_pwm_values_wave_form_t m_blinking_series[BLINKING_SERIES_LENGTH];

/*
 * Defines wave shape of a blink.
 */
static uint16_t blink_series_fn(uint16_t series_idx, uint16_t top_value)
{
  NRFX_ASSERT(series_idx < BLINKING_SERIES_LENGTH);

  const uint16_t PICK_SERIES_IDX = BLINKING_SERIES_LENGTH / 2;

  if (series_idx <= PICK_SERIES_IDX)
  {
    // [0, PICK_SERIES_IDX] -> [0, TOP_VALUE]
    return NRFX_ROUNDED_DIV(top_value * series_idx, PICK_SERIES_IDX);
  }
  else
  {
    // (PICK_SERIES_IDX, BLINKING_SERIES_LENGTH) -> (TOP_VALUE, 0)
    return NRFX_ROUNDED_DIV(top_value * (BLINKING_SERIES_LENGTH - series_idx), PICK_SERIES_IDX);
  }
}

static uint16_t calc_top_value_for_series_period_ms(uint16_t period_ms)
{
  // For PWM_MODE_UP:
  //         T(pwm) = T(pwm_clk) * TOP_VALUE
  //  =>  TOP_VALUE = Freq(pwm_clk) / Freq(pwm)

  // Also, Freq(pwm) = Length(series) / T(series)

  uint16_t pwm_frequency_hz = NRFX_ROUNDED_DIV(BLINKING_SERIES_LENGTH * 1000, period_ms);
  uint16_t top_value        = NRFX_ROUNDED_DIV(BLINKING_PWM_CLOCK_FREQUENCY_KHZ * 1000, pwm_frequency_hz);

  return top_value;
}

#define PWM_VALUES_COMMON_WAVE_FORM(common_pwm_value, top_value) \
  (nrf_pwm_values_wave_form_t)                                   \
  {                                                              \
    .channel_0   = common_pwm_value,                             \
    .channel_1   = common_pwm_value,                             \
    .channel_2   = common_pwm_value,                             \
    .counter_top = top_value,                                    \
  }

static void update_series_for_singular_intensity_percent(uint16_t intensity_percent)
{
  uint16_t pwm_value = NRFX_ROUNDED_DIV(TOP_VALUE_FOR_SINGLULAR_INTENSITY * intensity_percent, 100);

  for (size_t i = 0; i < BLINKING_SERIES_LENGTH; i++)
  {
    m_blinking_series[i] = PWM_VALUES_COMMON_WAVE_FORM(pwm_value, TOP_VALUE_FOR_SINGLULAR_INTENSITY);
  }
}

static void update_series_for_blink_period_ms(uint16_t period_ms)
{
  uint16_t top_value = calc_top_value_for_series_period_ms(period_ms);

  for (size_t i = 0; i < BLINKING_SERIES_LENGTH; i++)
  {
    uint16_t pwm_value = blink_series_fn(i, top_value);

    m_blinking_series[i] = PWM_VALUES_COMMON_WAVE_FORM(pwm_value, top_value);
  }
}

/*
 * Can be used for both restarting and starting the sequence.
 */
static void blinking_pwm_playback(blinking_led_t * led)
{
  nrfx_err_t err_code = nrfx_pwm_simple_playback(led->p_pwm_inst, &led->pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
  APP_ERROR_CHECK(err_code);
}

static void blinking_update_with_singular_intensity_percent(blinking_led_t * led, uint16_t intensity_percent)
{
  update_series_for_singular_intensity_percent(intensity_percent);

  // No matter if the LED is blinking or not,
  // There is no need to restart the sequence, new values will be applied immediately.

  led->is_blinking = false;
}

static void blinking_update_with_blink_period_ms(blinking_led_t * led, uint16_t period_ms)
{
  update_series_for_blink_period_ms(period_ms);

  if (led->is_blinking == false)
  {
    // PWM is playing series with same values to keep LED with single intensity.
    // This will make PWM skip remaining values in that series and start new one.

    // Maybe it causes double SEQSTART events (for same or different sequences),
    // if the singular series ends by itself.

    blinking_pwm_playback(led);
  }
  else
  {
    // Continue blinking with faster frequency, instead of restarting it.
  }

  led->is_blinking = true;
}

static void blinking_off(blinking_led_t * led)
{
  blinking_update_with_singular_intensity_percent(led, 0);
}

static void blinking_on(blinking_led_t * led)
{
  blinking_update_with_singular_intensity_percent(led, 100);
}

static void blinking_slow(blinking_led_t * led)
{
  blinking_update_with_blink_period_ms(led, BLINKING_MODE_SLOW_BLINK_PERIOD_MS);
}

static void blinking_fast(blinking_led_t * led)
{
  blinking_update_with_blink_period_ms(led, BLINKING_MODE_FAST_BLINK_PERIOD_MS);
}

void blinking_init(void)
{
  // Since all series values are 0 by default, there is not need of the following line:
  // blinking_off();
}

#define BLINKING_PWM_CONFIG(only_pin)                       \
  (nrfx_pwm_config_t)                                       \
  {                                                         \
    .output_pins =                                          \
    {                                                       \
      only_pin,                                             \
      NRFX_PWM_PIN_NOT_USED,                                \
      NRFX_PWM_PIN_NOT_USED,                                \
      NRFX_PWM_PIN_NOT_USED,                                \
    },                                                      \
    .base_clock   = BLINKING_PWM_CONFIG_BASE_CLOCK,         \
    .top_value    = TOP_VALUE_FOR_SINGLULAR_INTENSITY,      \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,   \
    .count_mode   = NRF_PWM_MODE_UP,                        \
    .load_mode    = NRF_PWM_LOAD_WAVE_FORM,                 \
    .step_mode    = NRF_PWM_STEP_AUTO,                      \
  }

#define BLINKING_PWM_SEQUENCE(p_wave_form_series)           \
  (nrf_pwm_sequence_t)                                      \
  {                                                         \
    .values.p_wave_form = m_blinking_series,                \
    .length             = BLINKING_SERIES_LENGTH * 4,       \
    .repeats            = 0,                                \
    .end_delay          = 0,                                \
  }

void blinking_enable(blinking_led_t * led)
{
  bsp_pin_no_t led_pin = c_bsp_board_led_idx_to_pin(led->led_idx);

  // currently this module does not support multi LEDs
  nrfx_pwm_config_t pwm_config = BLINKING_PWM_CONFIG(led_pin);

  led->pwm_seq = BLINKING_PWM_SEQUENCE(m_blinking_series);

  nrfx_err_t err_code = nrfx_pwm_init(led->p_pwm_inst, &pwm_config, NULL);
  APP_ERROR_CHECK(err_code);

  blinking_pwm_playback(led);
}

void blinking_set_mode(blinking_led_t * led, blinking_mode_t mode)
{
  NRF_LOG_INFO("[blinking]: switch to mode (%d)", mode);

  switch (mode)
  {
    case BLINKING_MODE_OFF:
      blinking_off(led);
      break;

    case BLINKING_MODE_ON:
      blinking_on(led);
      break;

    case BLINKING_MODE_BLINKS_SLOW:
      blinking_slow(led);
      break;

    case BLINKING_MODE_BLINKS_FAST:
      blinking_fast(led);
      break;
  }
}
