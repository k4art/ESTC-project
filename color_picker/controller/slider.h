#ifndef __SLIDER_H
#define __SLIDER_H

typedef enum sliding_mode_e
{
  SLIDING_MODE_CIRCULAR,
  SLIDING_MODE_UP_AND_DOWN,
} sliding_mode_t;

typedef enum sliding_dir_e
{
  SLIDING_DIR_UP,
  SLIDING_DIR_DOWN,
} sliding_dir_t;

struct slider_s;

typedef void (* slider_on_change_handler_fn)(struct slider_s * slider);

typedef struct slider_s
{
  sliding_mode_t sliding_mode;
  sliding_dir_t sliding_dir;
  uint16_t max_value;
  uint16_t current_value;
  uint16_t step_idx;
  slider_on_change_handler_fn on_change;
} slider_t;

#define SLIDING_FROM_BOTTOM(mode, max) \
{                                                         \
  .sliding_mode = mode,                                   \
  .sliding_dir = SLIDING_DIR_UP,                          \
  .max_value = max,                                       \
  .current_value = 0,                                     \
}

#define SLIDING_FROM_TOP(mode, max) \
{                                                      \
  .sliding_mode = mode,                                \
  .sliding_dir = SLIDING_DIR_DOWN,                     \
  .max_value = max,                                    \
  .current_value = max,                                \
}

void slider_init(void);

void slider_on_change(slider_t * ctx, slider_on_change_handler_fn fn);

void slider_start(slider_t * ctx);

void slider_stop(slider_t * ctx);

uint16_t slider_get_value(slider_t * ctx);

void slider_set_value(slider_t * ctx, uint16_t value);

#endif
