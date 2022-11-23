#include "utils.h"

#include "nrf_log.h"
#include "app_timer.h"

#include "xbutton/xbutton.h"

#include "color_picker/colors.h"
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

typedef struct color_picker_controller_control_block_s
{
  bool is_sliding;
  color_picker_controller_mode_t current_mode;
  color_picker_controller_hsv_handler_fn on_input_change;
  slider_t hsv_sliders[HSV_COMPONENTS_NUMBER];
} color_picker_controller_control_block_t;

static color_picker_controller_control_block_t m_cb =
{
  .hsv_sliders =
  {
    [H_COMPONENT_IDX] = SLIDER_FROM_BOTTOM(SLIDER_MODE_CIRCULAR, H_COMPONENT_TOP_VALUE),
    [S_COMPONENT_IDX] = SLIDER_FROM_TOP(SLIDER_MODE_BOUNCE, S_COMPONENT_TOP_VALUE),
    [V_COMPONENT_IDX] = SLIDER_FROM_TOP(SLIDER_MODE_BOUNCE, V_COMPONENT_TOP_VALUE),
  },
};

static void switch_mode(void)
{
  m_cb.current_mode = mode_followed_by(m_cb.current_mode);
}

static size_t get_current_mode_slider_idx(void)
{
  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      NRFX_ASSERT(false);
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H: return H_COMPONENT_IDX;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S: return S_COMPONENT_IDX;
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V: return V_COMPONENT_IDX;
  }

  NRFX_ASSERT(false);
  return 0;
}

static void current_slider_start(void)
{
  NRFX_ASSERT(m_cb.is_sliding == false);

  slider_start(&m_cb.hsv_sliders[get_current_mode_slider_idx()]);
  m_cb.is_sliding = true;
}

static void current_slider_stop(void)
{
  NRFX_ASSERT(m_cb.is_sliding);

  slider_stop(&m_cb.hsv_sliders[get_current_mode_slider_idx()]);
  m_cb.is_sliding = false;
}

static hsv_color_t current_hsv_color_input(void)
{
  uint8_t h = slider_get_value(&m_cb.hsv_sliders[H_COMPONENT_IDX]);
  uint8_t s = slider_get_value(&m_cb.hsv_sliders[S_COMPONENT_IDX]);
  uint8_t v = slider_get_value(&m_cb.hsv_sliders[V_COMPONENT_IDX]);

  return HSV_COLOR(h, s, v);
}

static void on_input_change_handler(slider_t * slider)
{
  hsv_color_t hsv_color = current_hsv_color_input();

  CALL_IF_NOT_NULL(m_cb.on_input_change, hsv_color);
}

static void current_slider_start_handler(uint8_t button_idx)
{
  (void) button_idx;

  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V:
      current_slider_start();
      break;
  }
}

static void current_slider_stop_handler(uint8_t button_idx)
{
  (void) button_idx;

  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V:
      current_slider_stop();
      break;
  }
}

static void switch_mode_handler(uint8_t button_idx)
{
  (void) button_idx;

  // handing this case would require extra logic
  NRFX_ASSERT(m_cb.is_sliding == false);

  switch_mode();
}

void color_picker_controller_init(void)
{
  xbutton_init();
  slider_init();

  for (size_t i = 0; i < NRFX_ARRAY_SIZE(m_cb.hsv_sliders); i++)
  {
    slider_on_change(&m_cb.hsv_sliders[i], on_input_change_handler);
  }
}

void color_picker_controller_set_hsv(hsv_color_t hsv)
{
  slider_set_value(&m_cb.hsv_sliders[H_COMPONENT_IDX], hsv.hue);
  slider_set_value(&m_cb.hsv_sliders[S_COMPONENT_IDX], hsv.saturation);
  slider_set_value(&m_cb.hsv_sliders[V_COMPONENT_IDX], hsv.value);
}

hsv_color_t color_picker_controller_get_hsv(void)
{
  return current_hsv_color_input();
}

void color_picker_controller_enable(uint8_t button_idx)
{
  bool high_accuracy = COLOR_PICKER_CONTROLLER_BUTTON_HIGH_ACCURACY != 0;

  nrfx_err_t err_code = xbutton_enable(button_idx, high_accuracy);
  APP_ERROR_CHECK(err_code);

  xbutton_on_double_click(button_idx, switch_mode_handler);

  xbutton_on_long_press_start(button_idx, current_slider_start_handler);
  xbutton_on_long_press_end(button_idx, current_slider_stop_handler);
}

void color_picker_controller_on_input_change_hsv(color_picker_controller_hsv_handler_fn handler)
{
  m_cb.on_input_change = handler;
}
