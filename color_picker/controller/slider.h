#ifndef __SLIDER_H
#define __SLIDER_H

#define SLIDER_REACHING_END_DURATION_MS  3000
#define SLIDER_INCREMENTING_FREQUENCY_HZ 100

#define SLIDER_INCREMENTING_PERIOD_MS    (1000 / SLIDER_INCREMENTING_FREQUENCY_HZ)
#define SLIDER_TOTAL_STEPS_NUMBER        (SLIDER_REACHING_END_DURATION_MS * SLIDER_INCREMENTING_FREQUENCY_HZ / 1000)
#define SLIDER_LAST_STEP_IDX             (SLIDER_TOTAL_STEPS_NUMBER - 1)

STATIC_ASSERT(SLIDER_LAST_STEP_IDX > 0);

typedef enum slider_mode_e
{
  SLIDER_MODE_CIRCULAR,
  SLIDER_MODE_BOUNCE,
} slider_mode_t;

typedef enum slider_direction_s
{
  SLIDER_DIRECTION_UP,
  SLIDER_DIRECTION_DOWN,
} slider_direction_t;

typedef struct slider_state_s
{
  slider_direction_t direction;
  uint16_t current_value;

  /* Used to enable integer-only (float-less) arithmetic for current_value computation */
  /* Ranges from 0 to SLIDER_LAST_STEP_IDX and maps to 0 to max_value */
  uint16_t step_idx;
} slider_state_t;

typedef struct slider_s slider_t;

typedef void (* slider_on_change_handler_fn)(slider_t * slider);

struct slider_s
{
  uint16_t max_value;
  slider_mode_t slider_mode;
  slider_state_t state;
  slider_on_change_handler_fn on_change;
};

#define SLIDER_FROM_BOTTOM(mode, max) \
{                                     \
  .slider_mode = mode,                \
  .max_value = max,                   \
  .state = {                          \
    .direction = SLIDER_DIRECTION_UP, \
    .current_value = 0,               \
    .step_idx = 0,                    \
  },                                  \
}

#define SLIDER_FROM_TOP(mode, max)      \
{                                       \
  .slider_mode = mode,                  \
  .max_value = max,                     \
  .state = {                            \
    .direction = SLIDER_DIRECTION_DOWN, \
    .current_value = max,               \
    .step_idx = SLIDER_LAST_STEP_IDX,   \
  },                                    \
}

void slider_init(void);

void slider_on_change(slider_t * slider, slider_on_change_handler_fn fn);

void slider_start(slider_t * slider);
void slider_stop(slider_t * slider);

uint16_t slider_get_value(slider_t * slider);
void slider_set_value(slider_t * slider, uint16_t value);

#endif
