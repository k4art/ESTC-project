#include "app_timer.h"
#include "nrf_log.h"

#include "gpio/c_bsp.h"
#include "utils.h"

#include "internals/btn_clickable.h"

#include "xbutton.h"

#define DOUBLE_CLICK_MAX_SECOND_CLICK_WAITING_TIME_MS 300

/*
 * State Transitions and Events Triggering:
 * |-----------------------------|-----------------------------|-----------------------------|-------------------------|-----------------|
 * | State \ Action              | RAW_CLICK                   | DOUBLE_CLICK_INTENT_TIMEOUT | LONG_PRESS_START        | LONG_PRESS_STOP |
 * |-----------------------------|-----------------------------|-----------------------------|-------------------------|-----------------|
 * | NEUTRAL                     | WAITING_DOUBLE_CLICK_INTENT | NEUTRAL                     | LONG_PRESSING           | Ignored         |
 * |                             | none                        | none                        | LONG_PRESS_START        |                 |
 * |-----------------------------|-----------------------------|-----------------------------|-------------------------|-----------------|
 * | WAITING_DOUBLE_CLICK_INTENT | NEUTRAL                     | NEUTRAL                     | LONG_PRESSING           | Ignored         |
 * |                             | DOUBLE_CLICK                | CLICK                       | CLICK, LONG_PRESS_START |                 |
 * |-----------------------------|-----------------------------|-----------------------------|-------------------------|-----------------|
 * | LONG_PRESSING               | Ignored                     | Ignored                     | Ignored                 | NEUTRAL         |
 * |                             |                             |                             |                         | LONG_PRESS_STOP |
 * |-----------------------------|-----------------------------|-----------------------------|-------------------------|-----------------|
 */

typedef enum
{
  BUTTON_STATE_NEUTRAL,
  BUTTON_STATE_WAITING_DOUBLE_CLICK_INTENT,
  BUTTON_STATE_LONG_PRESSING,
} btn_state_t;

typedef enum
{
  BUTTON_ACTION_RAW_CLICK,
  BUTTON_ACTION_DOUBLE_CLICK_INTENT_TIMEOUT,
  BUTTON_ACTION_LONG_PRESS_START,
  BUTTON_ACTION_LONG_PRESS_STOP,
} btn_action_t;

typedef enum
{
  BUTTON_EVENT_CLICK,
  BUTTON_EVENT_DOUBLE_CLICK,
  BUTTON_EVENT_LONG_PRESS_START,
  BUTTON_EVENT_LONG_PRESS_STOP,
} btn_event_t;

typedef struct btn_info_s
{
  bool is_used;
  btn_state_t state;

  xbutton_handler_t on_click;
  xbutton_handler_t on_double_click;
  xbutton_handler_t on_long_press_start;
  xbutton_handler_t on_long_press_stop;
} btn_info_t;

typedef struct xbutton_control_block_s
{
  btn_info_t btns[BUTTONS_NUMBER];
} xbutton_control_block_t;

static xbutton_control_block_t m_cb;

APP_TIMER_DEF(m_double_click_timeout_timer);

static bool btn_is_used(uint8_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));
  return m_cb.btns[button_idx].is_used;
}

static void emit_event(uint8_t button_idx, btn_event_t event)
{
  NRFX_ASSERT(btn_is_used(button_idx));

  switch (event)
  {
    case BUTTON_EVENT_CLICK:
      NRF_LOG_INFO("[xbutton]: %d - CLICK", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_click, button_idx);
      break;

    case BUTTON_EVENT_DOUBLE_CLICK:
      NRF_LOG_INFO("[xbutton]: %d - DOUBLE_CLICK", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_double_click, button_idx);
      break;

    case BUTTON_EVENT_LONG_PRESS_START:
      NRF_LOG_INFO("[xbutton]: %d - LONG_PRESS_START", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_long_press_start, button_idx);
      break;

    case BUTTON_EVENT_LONG_PRESS_STOP:
      NRF_LOG_INFO("[xbutton]: %d - LONG_PRESS_STOP", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_long_press_stop, button_idx);
      break;
  }
}

static void start_double_click_intent_timeout_timer(uint8_t button_idx)
{
  app_timer_start(m_double_click_timeout_timer,
                  APP_TIMER_TICKS(DOUBLE_CLICK_MAX_SECOND_CLICK_WAITING_TIME_MS),
                  (void *) (uint32_t) button_idx);
}

static void stop_double_click_intent_timeout_timer(void)
{
  app_timer_stop(m_double_click_timeout_timer);
}

static void button_fsm_next_state(uint8_t button_idx, btn_action_t action)
{
  NRFX_ASSERT(btn_is_used(button_idx));

  switch (m_cb.btns[button_idx].state)
  {
    case BUTTON_STATE_NEUTRAL:
      if (action == BUTTON_ACTION_RAW_CLICK)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_WAITING_DOUBLE_CLICK_INTENT;
        start_double_click_intent_timeout_timer(button_idx);
      }
      else if (action == BUTTON_ACTION_LONG_PRESS_START)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_LONG_PRESSING;
        emit_event(button_idx, BUTTON_EVENT_LONG_PRESS_START);
      }
      break;

    case BUTTON_STATE_WAITING_DOUBLE_CLICK_INTENT:
      if (action == BUTTON_ACTION_RAW_CLICK)
      {
        stop_double_click_intent_timeout_timer();

        m_cb.btns[button_idx].state = BUTTON_STATE_NEUTRAL;

        emit_event(button_idx, BUTTON_EVENT_DOUBLE_CLICK);
      }
      else if (action == BUTTON_ACTION_DOUBLE_CLICK_INTENT_TIMEOUT)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_NEUTRAL;

        emit_event(button_idx, BUTTON_EVENT_CLICK);
      }
      else if (action == BUTTON_ACTION_LONG_PRESS_START)
      {
        stop_double_click_intent_timeout_timer();

        m_cb.btns[button_idx].state = BUTTON_STATE_LONG_PRESSING;

        emit_event(button_idx, BUTTON_EVENT_CLICK);
        emit_event(button_idx, BUTTON_EVENT_LONG_PRESS_START);
      }
      break;

    case BUTTON_STATE_LONG_PRESSING:
      if (action == BUTTON_ACTION_LONG_PRESS_STOP)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_NEUTRAL;

        emit_event(button_idx, BUTTON_EVENT_LONG_PRESS_STOP);
      }
      break;
  }
}

static void handle_raw_click(uint8_t button_idx)
{
  NRFX_ASSERT(btn_is_used(button_idx));

  button_fsm_next_state(button_idx, BUTTON_ACTION_RAW_CLICK);
}

static void handle_timer_double_click_timeout(void * context)
{
  uint8_t button_idx = (uint32_t) context;
  NRFX_ASSERT(btn_is_used(button_idx));

  button_fsm_next_state(button_idx, BUTTON_ACTION_DOUBLE_CLICK_INTENT_TIMEOUT);
}

void xbutton_init(void)
{
  btn_clickable_init();

  ret_code_t ret  = app_timer_create(&m_double_click_timeout_timer,
                                     APP_TIMER_MODE_SINGLE_SHOT,
                                     handle_timer_double_click_timeout);

  APP_ERROR_CHECK(ret);
}

static void handle_long_press_start(uint8_t button_idx)
{
  button_fsm_next_state(button_idx, BUTTON_ACTION_LONG_PRESS_START);
}

static void handle_long_press_stop(uint8_t button_idx)
{
  button_fsm_next_state(button_idx, BUTTON_ACTION_LONG_PRESS_STOP);
}

nrfx_err_t xbutton_enable(uint8_t button_idx, bool high_accuracy)
{
  NRFX_ASSERT(!btn_is_used(button_idx));
  nrfx_err_t err_code = btn_clickable_enable(button_idx, high_accuracy);

  if (err_code == NRFX_SUCCESS)
  {
    m_cb.btns[button_idx].is_used = true;

    btn_clickable_on_click(button_idx, handle_raw_click);

    btn_clickable_on_long_press_start(button_idx, handle_long_press_start);
    btn_clickable_on_long_press_stop(button_idx, handle_long_press_stop);
  }

  return err_code;
}

#define SUPPORT_EVENT_REGISTER(event)                   \
  void xbutton_on_ ## event (uint8_t button_idx,        \
                             xbutton_handler_t handler) \
  {                                                     \
    NRFX_ASSERT(handler != NULL)                        \
    NRFX_ASSERT(btn_is_used(button_idx));               \
    m_cb.btns[button_idx].on_ ## event = handler;       \
  }

SUPPORT_EVENT_REGISTER(click)
SUPPORT_EVENT_REGISTER(double_click)

SUPPORT_EVENT_REGISTER(long_press_start)
SUPPORT_EVENT_REGISTER(long_press_stop)
