#include "app_timer.h"

#include "nrf_log.h"
#include "nrfx.h"

#include "utils.h"

#include "color_picker/controller/slider.h"

#define SLIDER_FULL_PERIOD_MS            3000
#define SLIDER_INCREMENTING_FREQUENCY_HZ 10

const uint32_t SLIDER_INCREMENTING_PERIOD_MS = 1000 / SLIDER_INCREMENTING_FREQUENCY_HZ;

const uint16_t SLIDER_TOTAL_STEPS_NUMBER = SLIDER_FULL_PERIOD_MS * SLIDER_INCREMENTING_FREQUENCY_HZ / 1000;

const uint16_t SLIDER_LAST_STEP_IDX = SLIDER_TOTAL_STEPS_NUMBER - 1;

APP_TIMER_DEF(m_sliding_timer);

static void switch_direction(slider_t * ctx)
{
  switch (ctx->sliding_dir)
  {
    case SLIDING_DIR_UP:
      ctx->sliding_dir = SLIDING_DIR_DOWN;
      break;

    case SLIDING_DIR_DOWN:
      ctx->sliding_dir = SLIDING_DIR_UP;
      break;
  }
}

static void slider_handle_reaching_max(slider_t * ctx)
{
  switch (ctx->sliding_mode)
  {
    case SLIDING_MODE_CIRCULAR:
      NRFX_ASSERT(ctx->sliding_dir == SLIDING_DIR_UP);

      ctx->step_idx = 0;
      break;

    case SLIDING_MODE_UP_AND_DOWN:
      ctx->step_idx = SLIDER_LAST_STEP_IDX;

      switch_direction(ctx);
      break;
  }
}

static void slider_handle_reaching_zero(slider_t * ctx)
{
  switch (ctx->sliding_mode)
  {
    case SLIDING_MODE_CIRCULAR:
      NRFX_ASSERT(ctx->sliding_dir == SLIDING_DIR_DOWN);

      ctx->step_idx = SLIDER_LAST_STEP_IDX;
      break;

    case SLIDING_MODE_UP_AND_DOWN:
      ctx->step_idx = 0;

      switch_direction(ctx);
      break;
  }
}

static void slider_next_step_idx(slider_t * ctx)
{
  switch (ctx->sliding_dir)
  {
    case SLIDING_DIR_UP:
      if (ctx->step_idx++ == SLIDER_LAST_STEP_IDX)
      {
        slider_handle_reaching_max(ctx);
      }
      break;

    case SLIDING_DIR_DOWN:
      if (ctx->step_idx-- == 0)
      {
        slider_handle_reaching_zero(ctx);
      }
      break;
  }
}

static void slider_increment(slider_t * ctx)
{
  slider_next_step_idx(ctx);

  // used uint32_t to avoid possible overflow during calculation
  ctx->current_value = (uint32_t) ctx->max_value * ctx->step_idx / SLIDER_LAST_STEP_IDX;
}

static void smooth_increment_handler(void * ctx)
{
  slider_t * slider = (slider_t *) ctx;

  slider_increment(slider);
  CALL_IF_NOT_NULL(slider->on_change, slider);
}

void slider_init(void)
{
  app_timer_init();

  app_timer_create(&m_sliding_timer, APP_TIMER_MODE_REPEATED, smooth_increment_handler);
}

void slider_on_change(slider_t * ctx, slider_on_change_handler_fn fn)
{
  ctx->on_change = fn;
}

void slider_start(slider_t * ctx)
{
  app_timer_start(m_sliding_timer,
                  APP_TIMER_TICKS(SLIDER_INCREMENTING_PERIOD_MS),
                  ctx);
}

void slider_stop(slider_t * ctx)
{
  app_timer_stop(m_sliding_timer);
}

uint16_t slider_get_value(slider_t * ctx)
{
  return ctx->current_value;
}

void slider_set_value(slider_t * ctx, uint16_t value)
{
  // used uint32_t to avoid possible overflow during calculation
  ctx->step_idx = (uint32_t) value * SLIDER_LAST_STEP_IDX / ctx->max_value;
  ctx->current_value = value; // skipping adjustment
}
