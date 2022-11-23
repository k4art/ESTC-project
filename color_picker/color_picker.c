#include "app_timer.h"

#include "leds/rgb_led_pwm.h"

#include "gpio/c_bsp.h"
#include "xbutton/xbutton.h"

#include "color_picker/controller/color_picker_controller.h"

#include "color_picker/color_picker.h"

#define INCREMENTING_PERIOD_MS 50
#define INCREMENTING_STEP       5

static rgb_led_pwm_color_t rgb_pwm_color = { .red = 100, .green = 0, .blue = 0 };
static nrfx_pwm_t pwm_inst = NRFX_PWM_INSTANCE(0);

static void copy_cntrl_value_to_rgb_pwm(rgb_color_t color)
{
  rgb_pwm_color.red   = color.red;
  rgb_pwm_color.green = color.green;
  rgb_pwm_color.blue  = color.blue;
}

void color_picker_init(void)
{
  c_bsp_board_init();
  color_picker_controller_init();
}

void color_picker_set_hsv(hsv_color_t hsv)
{
  color_picker_controller_set_hsv(hsv);
}

void color_picker_enable(uint8_t button_idx, uint8_t rgb_led_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));
  NRFX_ASSERT(IS_VALID_RGB_LED_IDX(rgb_led_idx));

  nrfx_err_t err_code = rgb_led_pwm_enable(rgb_led_idx, &pwm_inst, &rgb_pwm_color);
  NRFX_ASSERT(err_code == NRFX_SUCCESS);

  color_picker_controller_enable(USER_BUTTON_IDX);
  color_picker_controller_on_input_change_rgb(copy_cntrl_value_to_rgb_pwm);

  rgb_led_pwm_turn_on(rgb_led_idx);
}
