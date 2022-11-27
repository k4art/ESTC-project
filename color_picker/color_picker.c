#include "app_timer.h"

#include "leds/rgb_led.h"

#include "gpio/c_bsp.h"
#include "xbutton/xbutton.h"

#include "color_picker/controller/color_picker_controller.h"

#include "color_picker/color_picker.h"

#define COLOR_PICKER_RGB_LED_PWM_INSTANCE_ID    0
#define COLOR_PICKER_STATUS_LED_PWM_INSTANCE_ID 1

typedef struct color_picker_control_block_s
{
  bsp_idx_t rgb_led_idx;
  blinking_led_t status_blinking_led;
  nrfx_pwm_t rgb_led_pwm_inst;
  nrfx_pwm_t status_led_pwm_inst;
} color_picker_control_block_t;

static color_picker_control_block_t m_cb =
{
  .status_led_pwm_inst = NRFX_PWM_INSTANCE(COLOR_PICKER_STATUS_LED_PWM_INSTANCE_ID),
  .rgb_led_pwm_inst    = NRFX_PWM_INSTANCE(COLOR_PICKER_RGB_LED_PWM_INSTANCE_ID),
};

static void display_hsv_color(hsv_color_t hsv)
{
  rgb_color_t rgb = hsv_to_rgb(hsv);

  rgb_led_set_color(m_cb.rgb_led_idx, rgb);
}

static void hsv_color_input_change_handler(hsv_color_t hsv)
{
  display_hsv_color(hsv);
}

void color_picker_init(void)
{
  c_bsp_board_init();
  color_picker_controller_init();
}

/*
 * It does not pauses the color_picker_controller, so
 * it should be used before color_picker_enable(), or when user does not change color.
 */
void color_picker_set_hsv(hsv_color_t hsv)
{
  display_hsv_color(hsv);
  color_picker_controller_set_hsv(hsv);
}

void color_picker_enable(uint8_t button_idx, uint8_t rgb_led_idx, uint8_t status_led_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));

  rgb_led_enable(rgb_led_idx, &m_cb.rgb_led_pwm_inst);

  m_cb.rgb_led_idx = rgb_led_idx;
  m_cb.status_blinking_led = BLINKING_LED(status_led_idx, &m_cb.status_led_pwm_inst);

  blinking_enable(&m_cb.status_blinking_led);

  color_picker_controller_enable(USER_BUTTON_IDX, &m_cb.status_blinking_led);
  color_picker_controller_on_input_change_hsv(hsv_color_input_change_handler);
}
