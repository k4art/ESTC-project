#include "app_timer.h"
#include "nrf_log.h"
#include "nrfx_gpiote.h"

#include "gpio/c_bsp.h"
#include "utils.h"

#include "btn_debounced.h"

#include "btn_clickable.h"

/*
 * Maximum time from press to release to be counted as click.
 * If a button is not released after this amount of time,
 * the button is considered to be long pressing.
 */
#define MAX_CLICK_DURATION_MS 500

/*
 * State Transitions and Events Triggering:
 * |----------------------|----------------------|-------------------------|----------------------|
 * | State \ Action       | PRESS                | RELEASE                 | CLICK_INTENT_TIMEOUT |
 * |----------------------|----------------------|-------------------------|----------------------|
 * | NEUTRAL              | WAITING_CLICK_INTENT | Ignored                 | Ignored              |
 * |                      | PRESS                |                         |                      |
 * |----------------------|----------------------|-------------------------|----------------------|
 * | WAITING_CLICK_INTENT | Ignored              | NEUTRAL                 | LONG_PRESSING        |
 * |                      |                      | RELEASE, CLICK          | LONG_PRESS_START     |
 * |----------------------|----------------------|-------------------------|----------------------|
 * | LONG_PRESSING        | Ignored              | NEUTRAL                 | Ignored              |
 * |                      |                      | RELEASE, LONG_PRESS_END |                      |
 * |----------------------|----------------------|-------------------------|----------------------|
 */

typedef enum
{
  BUTTON_STATE_NEUTRAL,
  BUTTON_STATE_WAITING_CLICK_INTENT,
  BUTTON_STATE_LONG_PRESSING,
} btn_state_t;

typedef enum
{
  BUTTON_ACTION_PRESS,
  BUTTON_ACTION_RELEASE,
  BUTTON_ACTION_CLICK_INTENT_TIMEOUT,
} btn_action_t;

typedef enum
{
  BUTTON_EVENT_PRESS,
  BUTTON_EVENT_RELEASE,
  BUTTON_EVENT_CLICK,
  BUTTON_EVENT_LONG_PRESS_START,
  BUTTON_EVENT_LONG_PRESS_END,
} btn_event_t;

typedef struct btn_info_s
{
  btn_state_t state;

  btn_clickable_handler_t on_press;
  btn_clickable_handler_t on_release;
  btn_clickable_handler_t on_click;
  btn_clickable_handler_t on_long_press_start;
  btn_clickable_handler_t on_long_press_end;
} btn_info_t;

typedef struct
{
  btn_info_t btns[BUTTONS_NUMBER];
} btn_clickable_control_block_t;

static btn_clickable_control_block_t m_cb;

APP_TIMER_DEF(click_timeout_timer);

static void emit_event(uint8_t button_idx, btn_event_t event)
{
  switch (event)
  {
    case BUTTON_EVENT_PRESS:
      NRF_LOG_INFO("[btn_clickable]: [%d] => event:PRESS", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_press, button_idx);
      break;

    case BUTTON_EVENT_RELEASE:
      NRF_LOG_INFO("[btn_clickable]: [%d] => event:RELEASE", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_release, button_idx);
      break;

    case BUTTON_EVENT_CLICK:
      NRF_LOG_INFO("[btn_clickable]: [%d] => event:CLICK", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_click, button_idx);
      break;

    case BUTTON_EVENT_LONG_PRESS_START:
      NRF_LOG_INFO("[btn_clickable]: [%d] => event:LONG_PRESS_START", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_long_press_start, button_idx);
      break;

    case BUTTON_EVENT_LONG_PRESS_END:
      NRF_LOG_INFO("[btn_clickable]: [%d] => event:LONG_PRESS_END", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_long_press_end, button_idx);
      break;
  }
}

static void start_click_intent_timeout_timer(uint8_t button_idx)
{
  app_timer_start(click_timeout_timer,
                  APP_TIMER_TICKS(MAX_CLICK_DURATION_MS),
                  (void *) (uint32_t) button_idx);
}

static void button_fsm_next_state(uint8_t button_idx,
                                 btn_action_t action)
{
  switch (m_cb.btns[button_idx].state)
  {
    case BUTTON_STATE_NEUTRAL:
      if (action == BUTTON_ACTION_PRESS)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_WAITING_CLICK_INTENT;

        start_click_intent_timeout_timer(button_idx);
        emit_event(button_idx, BUTTON_EVENT_PRESS);
      }
      break;

    case BUTTON_STATE_WAITING_CLICK_INTENT:
      if (action == BUTTON_ACTION_RELEASE)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_NEUTRAL;

        emit_event(button_idx, BUTTON_EVENT_RELEASE);
        emit_event(button_idx, BUTTON_EVENT_CLICK);
      }
      else if (action == BUTTON_ACTION_CLICK_INTENT_TIMEOUT)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_LONG_PRESSING;

        emit_event(button_idx, BUTTON_EVENT_LONG_PRESS_START);
      }
      break;

    case BUTTON_STATE_LONG_PRESSING:
      if (action == BUTTON_ACTION_RELEASE)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_NEUTRAL;

        emit_event(button_idx, BUTTON_EVENT_RELEASE);
        emit_event(button_idx, BUTTON_EVENT_LONG_PRESS_END);
      }
      break;
  }
}

static void handle_press(uint8_t button_idx)
{
  button_fsm_next_state(button_idx, BUTTON_ACTION_PRESS);
}

static void handle_release(uint8_t button_idx)
{
  app_timer_stop(click_timeout_timer);

  button_fsm_next_state(button_idx, BUTTON_ACTION_RELEASE);
}

static void handle_click_timeout_timer(void * context)
{
  button_fsm_next_state((uint32_t) context, BUTTON_ACTION_CLICK_INTENT_TIMEOUT);
}

void btn_clickable_init(void)
{btn_debounced_init();

  ret_code_t ret = app_timer_create(&click_timeout_timer,
                                    APP_TIMER_MODE_SINGLE_SHOT,
                                    handle_click_timeout_timer);

  APP_ERROR_CHECK(ret);
}

nrfx_err_t btn_clickable_enable(uint8_t button_idx, bool high_accuracy)
{
  nrfx_err_t err_code = btn_debounced_enable(button_idx, high_accuracy);

  if (err_code == NRFX_SUCCESS)
  {
    btn_debounced_on_press(button_idx, handle_press);
    btn_debounced_on_release(button_idx, handle_release);
  }

  return err_code;
}

#define SUPPORT_EVENT_REGISTER(event)                               \
  void btn_clickable_on_ ## event (uint8_t button_idx,              \
                                   btn_clickable_handler_t handler) \
  {                                                                 \
    NRFX_ASSERT(handler != NULL)                                    \
    NRFX_ASSERT(btn_debounced_is_enabled_for(button_idx));          \
    m_cb.btns[button_idx].on_ ## event = handler;                   \
  }

SUPPORT_EVENT_REGISTER(press)
SUPPORT_EVENT_REGISTER(release)
SUPPORT_EVENT_REGISTER(click)
SUPPORT_EVENT_REGISTER(long_press_start)
SUPPORT_EVENT_REGISTER(long_press_end)
