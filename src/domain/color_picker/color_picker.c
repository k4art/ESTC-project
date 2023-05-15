#include "app_util.h"
#include "nrf_log.h"
#include "app_timer.h"

#include "lib/gpio/c_bsp.h"

#include "io/leds/rgb_led.h"
#include "io/xbutton/xbutton.h"

#include "flash/fappdata.h"
#include "flash/fvarstorage/fvarstorage.h"

#include "domain/color_picker/controller/color_picker_controller.h"
#include "domain/color_picker/cli/color_picker_cli.h"

#include "domain/color_picker/color_picker.h"
#include "domain/events/events.h"

#define COLOR_PICKER_RGB_LED_PWM_INSTANCE_ID    0
#define COLOR_PICKER_STATUS_LED_PWM_INSTANCE_ID 1
#define COLOR_PICKER_FAPPDATA_COLOR_VARIABLE_ID 0x1

typedef struct color_picker_control_block_s
{
  rgb_led_t         viewer_color_rgb_led;
  blinking_led_t    status_blinking_led;
  nrfx_pwm_t        rgb_led_pwm_inst;
  nrfx_pwm_t        status_led_pwm_inst;
  fvarstorage_t     fstorage;
} color_picker_control_block_t;

static color_picker_control_block_t m_cb =
{
  .status_led_pwm_inst = NRFX_PWM_INSTANCE(COLOR_PICKER_STATUS_LED_PWM_INSTANCE_ID),
  .rgb_led_pwm_inst    = NRFX_PWM_INSTANCE(COLOR_PICKER_RGB_LED_PWM_INSTANCE_ID),
};

static void notify_app_events(app_events_evt_t evt, rgb_color_t rgb)
{
  app_events_emit(evt, rgb_to_word(rgb));
}

static void display_hsv_color(hsv_color_t hsv)
{
  rgb_color_t rgb = hsv_to_rgb(hsv);

  notify_app_events(APP_EVENTS_EVT_LED_SYNC, rgb);
  rgb_led_set_color(&m_cb.viewer_color_rgb_led, rgb);
}

static void hsv_color_input_change_handler(hsv_color_t hsv)
{
  display_hsv_color(hsv);
}

static void save_hsv(hsv_color_t hsv)
{
  uint32_t word = hsv_to_word(hsv);
  fvarstorage_save(&m_cb.fstorage, &word);
}

static void sync_color_hsv(app_events_evt_t evt, uint32_t word)
{
  save_hsv(word_to_hsv(word));
}

void color_picker_init(void)
{
  color_picker_controller_init();
  
  fvarstorage_init(&m_cb.fstorage,
                   COLOR_PICKER_FAPPDATA_COLOR_VARIABLE_ID,
                   FAPPDATA_PAGE_MASK_BY_IDX(0) | FAPPDATA_PAGE_MASK_BY_IDX(1));

#if NRFX_CHECK(ESTC_USB_CLI_ENABLED)
  color_picker_cli_register();
#endif
}

/*
 * It does not pauses the color_picker_controller, so
 * it should be used before color_picker_enable(), or when user does not change color.
 */
void color_picker_set_hsv(hsv_color_t hsv)
{
  display_hsv_color(hsv);
  color_picker_controller_set_hsv(hsv);

  app_events_emit(APP_EVENTS_EVT_COLOR_PICKER_SYNC, hsv_to_word(hsv));
}

void color_picker_restore_or_set_default_hsv(hsv_color_t hsv)
{
  uint32_t hsv_color_word;

  if (fvarstorage_restore(&m_cb.fstorage, &hsv_color_word))
  {
    hsv = word_to_hsv(hsv_color_word);
  }

  color_picker_set_hsv(hsv);
  notify_app_events(APP_EVENTS_EVT_LED_SYNC, hsv_to_rgb(hsv));
}

void color_picker_enable(bsp_idx_t button_idx, bsp_idx_t rgb_led_idx, bsp_idx_t status_led_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));

  rgb_led_enable(&m_cb.viewer_color_rgb_led, rgb_led_idx, &m_cb.rgb_led_pwm_inst);
  blinking_led_enable(&m_cb.status_blinking_led, status_led_idx, &m_cb.status_led_pwm_inst);

  color_picker_controller_enable(USER_BUTTON_IDX, &m_cb.status_blinking_led);
  color_picker_controller_on_input_change_hsv(hsv_color_input_change_handler);
  color_picker_controller_on_edit_end(save_hsv);

  app_events_on(APP_EVENTS_EVT_COLOR_PICKER_SYNC, sync_color_hsv);
}
