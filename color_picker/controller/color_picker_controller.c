#include "utils.h"

#include "nrf_log.h"
#include "app_timer.h"

#include "xbutton/xbutton.h"

#include "leds/blinking_led.h"

#include "color_picker/controller/slider.h"

#include "color_picker/controller/color_picker_controller.h"

#define COLOR_PICKER_CONTROLLER_BUTTON_HIGH_ACCURACY 1

typedef enum color_picker_controller_mode_e
{
  COLOR_PICKER_CONTROLLER_MODE_VIEWER,
  COLOR_PICKER_CONTROLLER_MODE_EDITOR_H,
  COLOR_PICKER_CONTROLLER_MODE_EDITOR_S,
  COLOR_PICKER_CONTROLLER_MODE_EDITOR_V,
} color_picker_controller_mode_t;

/*
 * Translations of color_picker_controller modes:
 *
 * VIEWER -> EDITOR_H -> EDITOR_S -> EDITOR_V
 */
static color_picker_controller_mode_t mode_followed_by(color_picker_controller_mode_t mode)
{
  switch (mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:   return COLOR_PICKER_CONTROLLER_MODE_EDITOR_H;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H: return COLOR_PICKER_CONTROLLER_MODE_EDITOR_S;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S: return COLOR_PICKER_CONTROLLER_MODE_EDITOR_V;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V: return COLOR_PICKER_CONTROLLER_MODE_VIEWER;
  }

  UNREACHABLE_RETURN(0);
}

/*
 * Corresponding blinking_led_mode for each color_picker_controller mode:
 *
 * |-------------------|-----------------|
 * |  Controller Mode  | Status LED Mode |
 * |-------------------|-----------------|
 * |      VIEWER       |  OFF            |
 * |      EDITOR_H     |  BLINKS_SLOW    |
 * |      EDITOR_S     |  BLINKS_FAST    |
 * |      EDITOR_V     |  ON             |
 * |-------------------|-----------------|
 */
static blinking_led_mode_t status_led_mode_for_controller_mode(color_picker_controller_mode_t mode)
{
  switch (mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:   return BLINKING_LED_MODE_OFF;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H: return BLINKING_LED_MODE_BLINKS_SLOW;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S: return BLINKING_LED_MODE_BLINKS_FAST;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V: return BLINKING_LED_MODE_ON;
  }

  UNREACHABLE_RETURN(0);
}

#define H_SLIDER_IDX 0
#define S_SLIDER_IDX 1
#define V_SLIDER_IDX 2

typedef struct color_picker_controller_control_block_s
{
  bool is_sliding;
  blinking_led_t * p_status_led;
  color_picker_controller_mode_t current_mode;
  color_picker_controller_hsv_handler_t on_input_change;
  color_picker_controller_hsv_handler_t on_edit_end;
  slider_t hsv_sliders[HSV_COMPONENTS_NUMBER];
} color_picker_controller_control_block_t;

static color_picker_controller_control_block_t m_cb =
{
  .hsv_sliders =
  {
    [H_SLIDER_IDX] = SLIDER_FROM_BOTTOM(SLIDER_MODE_CIRCULAR, H_COMPONENT_TOP_VALUE),
    [S_SLIDER_IDX] = SLIDER_FROM_TOP(SLIDER_MODE_BOUNCE, S_COMPONENT_TOP_VALUE),
    [V_SLIDER_IDX] = SLIDER_FROM_TOP(SLIDER_MODE_BOUNCE, V_COMPONENT_TOP_VALUE),
  },
};

static size_t get_current_slider_idx(void)
{
  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      NRFX_ASSERT(false);
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H: return H_SLIDER_IDX;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S: return S_SLIDER_IDX;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V: return V_SLIDER_IDX;
  }

  UNREACHABLE_RETURN(0);
}

static slider_t * get_slider_at(size_t slider_idx)
{
  return &m_cb.hsv_sliders[slider_idx];
}

static void start_current_slider(void)
{
  NRFX_ASSERT(m_cb.is_sliding == false);

  bsp_idx_t current_idx = get_current_slider_idx();

  slider_start(get_slider_at(current_idx));
  m_cb.is_sliding = true;
}

static void stop_current_slider(void)
{
  NRFX_ASSERT(m_cb.is_sliding);

  bsp_idx_t current_idx = get_current_slider_idx();

  slider_stop(get_slider_at(current_idx));
  m_cb.is_sliding = false;
}

static hsv_color_t current_hsv_color_input(void)
{
  uint8_t h = slider_get_value(get_slider_at(H_SLIDER_IDX));
  uint8_t s = slider_get_value(get_slider_at(S_SLIDER_IDX));
  uint8_t v = slider_get_value(get_slider_at(V_SLIDER_IDX));

  return HSV_COLOR(h, s, v);
}

static void switch_mode(void)
{
  m_cb.current_mode = mode_followed_by(m_cb.current_mode);
  blinking_led_set_mode(m_cb.p_status_led, status_led_mode_for_controller_mode(m_cb.current_mode));

  if (m_cb.current_mode == COLOR_PICKER_CONTROLLER_MODE_VIEWER)
  {
    hsv_color_t color = current_hsv_color_input();
    CALL_IF_NOT_NULL(m_cb.on_edit_end, color);
  }
}

static void on_input_change_handler(slider_t * slider)
{
  hsv_color_t hsv_color = current_hsv_color_input();

  CALL_IF_NOT_NULL(m_cb.on_input_change, hsv_color);
}

static void start_current_slider_handler(uint8_t button_idx)
{
  (void) button_idx;

  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V:
      start_current_slider();
      break;
  }
}

static void stop_current_slider_handler(uint8_t button_idx)
{
  (void) button_idx;

  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V:
      stop_current_slider();
      break;
  }
}

/*
 * Does not handle case when is_sliding == true.
 */
static void switch_mode_handler(uint8_t button_idx)
{
  (void) button_idx;

  NRFX_ASSERT(m_cb.is_sliding == false);

  switch_mode();
}

void color_picker_controller_init(void)
{
  xbutton_init();
  slider_init();

  for (size_t i = 0; i < NRFX_ARRAY_SIZE(m_cb.hsv_sliders); i++)
  {
    slider_on_change(get_slider_at(i), on_input_change_handler);
  }
}

void color_picker_controller_set_hsv(hsv_color_t hsv)
{
  slider_set_value(get_slider_at(H_SLIDER_IDX), hsv.hue);
  slider_set_value(get_slider_at(S_SLIDER_IDX), hsv.saturation);
  slider_set_value(get_slider_at(V_SLIDER_IDX), hsv.value);
}

hsv_color_t color_picker_controller_get_hsv(void)
{
  return current_hsv_color_input();
}

void color_picker_controller_enable(uint8_t button_idx, blinking_led_t * p_status_led)
{
  bool high_accuracy = COLOR_PICKER_CONTROLLER_BUTTON_HIGH_ACCURACY != 0;

  nrfx_err_t err_code = xbutton_enable(button_idx, high_accuracy);
  APP_ERROR_CHECK(err_code);

  xbutton_on_double_click(button_idx, switch_mode_handler);

  xbutton_on_long_press_start(button_idx, start_current_slider_handler);
  xbutton_on_long_press_stop(button_idx, stop_current_slider_handler);

  m_cb.p_status_led = p_status_led;
}

void color_picker_controller_on_input_change_hsv(color_picker_controller_hsv_handler_t handler)
{
  m_cb.on_input_change = handler;
}

void color_picker_controller_on_edit_end(color_picker_controller_hsv_handler_t handler)
{
  m_cb.on_edit_end = handler;
}
