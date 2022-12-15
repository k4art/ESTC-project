#include "app_timer.h"

#include "nrf_log.h"
#include "nrfx.h"

#include "utils.h"

#include "color_picker/controller/slider.h"

APP_TIMER_DEF(m_update_value_timer);

static slider_direction_t opposite_direction(slider_direction_t dir)
{
  switch (dir)
  {
    case SLIDER_DIRECTION_UP:   return SLIDER_DIRECTION_DOWN;
    case SLIDER_DIRECTION_DOWN: return SLIDER_DIRECTION_UP;
  }

  UNREACHABLE_RETURN(0);
}

static void switch_direction(slider_t * slider)
{
  slider->state.direction = opposite_direction(slider->state.direction);
}

static void handle_when_reaches_max(slider_t * slider)
{
  NRFX_ASSERT(slider->state.direction == SLIDER_DIRECTION_UP);

  switch (slider->slider_mode)
  {
    case SLIDER_MODE_CIRCULAR:
      slider->state.step_idx = 0;
      break;

    case SLIDER_MODE_BOUNCE:
      slider->state.step_idx = SLIDER_LAST_STEP_IDX;

      switch_direction(slider);
      break;
  }
}

static void handle_when_reaches_zero(slider_t * slider)
{
  NRFX_ASSERT(slider->state.direction == SLIDER_DIRECTION_DOWN);

  switch (slider->slider_mode)
  {
    case SLIDER_MODE_CIRCULAR:
      slider->state.step_idx = SLIDER_LAST_STEP_IDX;
      break;

    case SLIDER_MODE_BOUNCE:
      slider->state.step_idx = 0;

      switch_direction(slider);
      break;
  }
}

static void slider_next_step_idx(slider_t * slider)
{
  switch (slider->state.direction)
  {
    case SLIDER_DIRECTION_UP:
      if (slider->state.step_idx++ == SLIDER_LAST_STEP_IDX)
      {
        handle_when_reaches_max(slider);
      }
      break;

    case SLIDER_DIRECTION_DOWN:
      if (slider->state.step_idx-- == 0)
      {
        handle_when_reaches_zero(slider);
      }
      break;
  }
}

static void slider_increment(slider_t * slider)
{
  slider_next_step_idx(slider);
}

static void update_value_timer_handler(void * context)
{
  slider_t * slider = (slider_t *) context;

  slider_increment(slider);
  CALL_IF_NOT_NULL(slider->on_change, slider);
}

void slider_init(void)
{
  ret_code_t err = app_timer_create(&m_update_value_timer, APP_TIMER_MODE_REPEATED, update_value_timer_handler);
  APP_ERROR_CHECK(err);
}

void slider_start(slider_t * slider)
{
  ret_code_t err = app_timer_start(m_update_value_timer,
                                   APP_TIMER_TICKS(SLIDER_INCREMENTING_PERIOD_MS),
                                   slider);

  APP_ERROR_CHECK(err);
}

void slider_stop(slider_t * slider)
{
  ret_code_t err = app_timer_stop(m_update_value_timer);
  APP_ERROR_CHECK(err);
}

uint16_t slider_get_value(slider_t * slider)
{
  return NRFX_ROUNDED_DIV((uint32_t) slider->max_value * slider->state.step_idx, SLIDER_LAST_STEP_IDX);
}

void slider_set_value(slider_t * slider, uint16_t value)
{
  slider->state.step_idx = NRFX_ROUNDED_DIV((uint32_t) value * SLIDER_LAST_STEP_IDX, slider->max_value);
}

void slider_on_change(slider_t * slider, slider_on_change_handler_t fn)
{
  slider->on_change = fn;
}
