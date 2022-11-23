#include "utils.h"

#include "nrf_log.h"
#include "app_timer.h"

#include "xbutton/xbutton.h"

#include "color_picker/colors.h"
#include "color_picker/controller/slider.h"

#include "color_picker/controller/color_picker_controller.h"

#define COLOR_PICKER_CONTROLLER_BUTTON_HIGH_ACCURACY             1

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
 * VIEWER -> H_CHANGING -> S_CHANGING -> V_CHANGING
 */
static color_picker_controller_mode_t color_picker_controller_mode_translations[] =
{
  [COLOR_PICKER_CONTROLLER_MODE_VIEWER]   = COLOR_PICKER_CONTROLLER_MODE_EDITOR_H,
  [COLOR_PICKER_CONTROLLER_MODE_EDITOR_H] = COLOR_PICKER_CONTROLLER_MODE_EDITOR_S,
  [COLOR_PICKER_CONTROLLER_MODE_EDITOR_S] = COLOR_PICKER_CONTROLLER_MODE_EDITOR_V,
  [COLOR_PICKER_CONTROLLER_MODE_EDITOR_V] = COLOR_PICKER_CONTROLLER_MODE_VIEWER,
};

typedef struct color_picker_controller_control_block_s
{
  bool is_sliding;
  color_picker_controller_mode_t current_mode;
  slider_t hsv_sliders[HSV_COMPONENTS_NUMBER];
  color_picker_controller_rgb_handler input_change_rgb_handler;
} color_picker_controller_control_block_t;

static color_picker_controller_control_block_t m_cb =
{
  .hsv_sliders =
  {
    [H_COMPONENT_IDX] = SLIDING_FROM_BOTTOM(SLIDING_MODE_CIRCULAR,
                                            H_COMPONENT_TOP_VALUE),

    [S_COMPONENT_IDX] = SLIDING_FROM_TOP(SLIDING_MODE_UP_AND_DOWN,
                                         S_COMPONENT_TOP_VALUE),

    [V_COMPONENT_IDX] = SLIDING_FROM_TOP(SLIDING_MODE_UP_AND_DOWN,
                                         V_COMPONENT_TOP_VALUE),
  },
};

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

static void color_picker_controller_next_mode(uint8_t button_idx)
{
  NRFX_ASSERT(m_cb.is_sliding == false);

  m_cb.current_mode = color_picker_controller_mode_translations[m_cb.current_mode];
}

static void color_picker_controller_current_slider_start(void)
{
  NRFX_ASSERT(m_cb.is_sliding == false);

  slider_start(&m_cb.hsv_sliders[get_current_mode_slider_idx()]);
  m_cb.is_sliding = true;
}

static void color_picker_controller_current_slider_stop(void)
{
  NRFX_ASSERT(m_cb.is_sliding);

  slider_stop(&m_cb.hsv_sliders[get_current_mode_slider_idx()]);
  m_cb.is_sliding = false;
}

static void on_input_change_handler(slider_t * slider)
{
  hsv_color_t hsv = HSV_COLOR(m_cb.hsv_sliders[H_COMPONENT_IDX].current_value,
                              m_cb.hsv_sliders[S_COMPONENT_IDX].current_value,
                              m_cb.hsv_sliders[V_COMPONENT_IDX].current_value);

  CALL_IF_NOT_NULL(m_cb.input_change_rgb_handler, hsv_to_rgb(hsv));
}

static void sliding_start(uint8_t button_idx)
{
  (void) button_idx;

  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V:
      color_picker_controller_current_slider_start();
      break;
  }
}

static void sliding_stop(uint8_t button_idx)
{
  (void) button_idx;

  switch (m_cb.current_mode)
  {
    case COLOR_PICKER_CONTROLLER_MODE_VIEWER:
      break;

    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_H:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_S:
    case COLOR_PICKER_CONTROLLER_MODE_EDITOR_V:
      color_picker_controller_current_slider_stop();
      break;
  }
}

void color_picker_controller_init(void)
{
  xbutton_init();
  slider_init();

  slider_on_change(&m_cb.hsv_sliders[H_COMPONENT_IDX], on_input_change_handler);
  slider_on_change(&m_cb.hsv_sliders[S_COMPONENT_IDX], on_input_change_handler);
  slider_on_change(&m_cb.hsv_sliders[V_COMPONENT_IDX], on_input_change_handler);
}

void color_picker_controller_set_hsv(hsv_color_t hsv)
{
  slider_set_value(&m_cb.hsv_sliders[H_COMPONENT_IDX], hsv.hue);
  slider_set_value(&m_cb.hsv_sliders[S_COMPONENT_IDX], hsv.saturation);
  slider_set_value(&m_cb.hsv_sliders[V_COMPONENT_IDX], hsv.value);
}

void color_picker_controller_enable(uint8_t button_idx)
{
  bool high_accuracy = COLOR_PICKER_CONTROLLER_BUTTON_HIGH_ACCURACY != 0;

  nrfx_err_t err_code = xbutton_enable(button_idx, high_accuracy);
  APP_ERROR_CHECK(err_code);

  xbutton_on_double_click(button_idx, color_picker_controller_next_mode);

  xbutton_on_long_press_start(button_idx, sliding_start);
  xbutton_on_long_press_end(button_idx, sliding_stop);
}

void color_picker_controller_on_input_change_rgb(color_picker_controller_rgb_handler handler)
{
  // assert it is inited
  m_cb.input_change_rgb_handler = handler;
}
