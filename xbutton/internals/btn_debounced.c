#include "app_timer.h"
#include "nrfx_gpiote.h"
#include "nrf_log.h"

#include "gpio/c_bsp.h"
#include "utils.h"

#include "btn_debounced.h"

#define GPIOTE_CONFIG_IN_SENSE_DEF(var, polarity_in_caps, hi_acc) \
  nrfx_gpiote_in_config_t var = NRFX_GPIOTE_CONFIG_IN_SENSE_ ## polarity_in_caps(hi_acc); \
  var.pull = BUTTON_PULL

/*
 * State Transitions and Events Triggering:
 * |----------------|------------|---------|------------------|-------------------|
 * | State \ Action | PRESS      | RELEASE | DEBOUNCE_PRESSED | DEBOUNCE_RELEASED |
 * |----------------|------------|---------|------------------|-------------------|
 * | UP             | DEBOUNCING | Ignored | Ignored          | Ignored           |
 * |                | PRESS      |         |                  |                   |
 * |----------------|------------|---------|------------------|-------------------|
 * | DOWN           | Ignored    | UP      | Ignored          | Ignored           |
 * |                |            | RELEASE |                  |                   |
 * |----------------|------------|---------|------------------|-------------------|
 * | DEBOUNCING     | Ignored    | Ignored | DOWN             | UP                |
 * |                |            |         |                  | RELEASE           |
 * |----------------|------------|---------|------------------|-------------------|
 */

typedef enum
{
  BUTTON_STATE_UP,
  BUTTON_STATE_DOWN,
  BUTTON_STATE_DEBOUNCING,
} btn_state_t;

typedef enum
{
  BUTTON_ACTION_RELEASE,
  BUTTON_ACTION_PRESS,
  BUTTON_ACTION_DEBOUNCE_PRESSED,
  BUTTON_ACTION_DEBOUNCE_RELEASED,
} btn_action_t;

typedef enum
{
  BUTTON_EVENT_PRESS,
  BUTTON_EVENT_RELEASE,
} btn_event_t;

typedef struct btn_info_s
{
  bool is_used;
  btn_state_t state;
  btn_debounced_handler_t on_press;
  btn_debounced_handler_t on_release;
} btn_info_t;

typedef struct btn_debounced_control_block_s
{
  btn_info_t btns[BUTTONS_NUMBER];
} btn_debounced_control_block_t;

static btn_debounced_control_block_t m_cb;

APP_TIMER_DEF(m_debounce_timeout_timer);

static bool btn_is_used(uint8_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));
  return m_cb.btns[button_idx].is_used;
}

static void emit_event(uint8_t button_idx, btn_event_t event)
{
  switch (event)
  {
    case BUTTON_EVENT_PRESS:
      NRF_LOG_INFO("[btn_debounced]: [%d] - event press", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_press, button_idx);
      break;

    case BUTTON_EVENT_RELEASE:
      NRF_LOG_INFO("[btn_debounced]: [%d] - event release", button_idx);
      CALL_IF_NOT_NULL(m_cb.btns[button_idx].on_release, button_idx);
      break;
  }
}

static void start_debounce_timeout_timer(uint8_t button_idx)
{
  void * context = (void *) (uint32_t) button_idx;

  app_timer_start(m_debounce_timeout_timer,
                  APP_TIMER_TICKS(BUTTON_BOUNCING_TIME_MS),
                  context);
}

static void button_fsm_next_state(uint8_t button_idx, btn_action_t action)
{
  switch (m_cb.btns[button_idx].state) // previous state
  {
    case BUTTON_STATE_UP:
      if (action == BUTTON_ACTION_PRESS)
      {
        NRF_LOG_INFO("[btn_debounced]: [%d] - action press", button_idx);

        m_cb.btns[button_idx].state = BUTTON_STATE_DEBOUNCING;
        start_debounce_timeout_timer(button_idx);
        emit_event(button_idx, BUTTON_EVENT_PRESS);
      }
      break;

    case BUTTON_STATE_DOWN:
      if (action == BUTTON_ACTION_RELEASE)
      {
        NRF_LOG_INFO("[btn_debounced]: [%d] - action release", button_idx);

        m_cb.btns[button_idx].state = BUTTON_STATE_DEBOUNCING;
        start_debounce_timeout_timer(button_idx);
        emit_event(button_idx, BUTTON_EVENT_RELEASE);
      }
      break;

    case BUTTON_STATE_DEBOUNCING:
      if (action == BUTTON_ACTION_DEBOUNCE_PRESSED)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_DOWN;
      }
      else if (action == BUTTON_ACTION_DEBOUNCE_RELEASED)
      {
        m_cb.btns[button_idx].state = BUTTON_STATE_UP;
      }
      else
      {
        NRF_LOG_INFO("[btn_debounced]: [%d] - action was debounced (ignored)", button_idx);
      }
      break;
  }
}

static void click_intente_timeout_timer_handler(void * context)
{
  uint8_t button_idx = (uint32_t) context;

  if (c_bsp_board_button_state_get(button_idx) == 1)
  {
    button_fsm_next_state(button_idx, BUTTON_ACTION_DEBOUNCE_PRESSED);
  }
  else
  {
    button_fsm_next_state(button_idx, BUTTON_ACTION_DEBOUNCE_RELEASED);
  }
}

static void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  uint8_t button_idx = c_bsp_board_pin_to_button_idx(pin);

  NRFX_ASSERT(btn_is_used(button_idx));

  bool button_is_down = c_bsp_board_button_state_get(button_idx) == 1;

  if (button_is_down)
  {
    button_fsm_next_state(button_idx, BUTTON_ACTION_PRESS);
  }
  else
  {
    button_fsm_next_state(button_idx, BUTTON_ACTION_RELEASE);
  }
}

static nrfx_err_t gpiote_init_button(uint8_t button_idx, bool hi_acc)
{
  uint32_t button_pin = c_bsp_board_button_idx_to_pin(button_idx);

  GPIOTE_CONFIG_IN_SENSE_DEF(config, TOGGLE, hi_acc);

  nrfx_err_t err_code = nrfx_gpiote_in_init(button_pin, &config, gpiote_event_handler);

  nrfx_gpiote_in_event_enable(button_pin, true);

  return err_code;
}

void btn_debounced_init(void)
{
  ret_code_t ret = app_timer_init();
  APP_ERROR_CHECK(ret);

  if (!nrfx_gpiote_is_init())
  {
    nrfx_err_t err = nrfx_gpiote_init();
    NRFX_ASSERT(err == NRFX_SUCCESS);
  }

  ret = app_timer_create(&m_debounce_timeout_timer, APP_TIMER_MODE_SINGLE_SHOT, click_intente_timeout_timer_handler);
  APP_ERROR_CHECK(ret);
}

nrfx_err_t btn_debounced_enable(uint8_t button_idx, bool high_accuracy)
{
  NRFX_ASSERT(!btn_is_used(button_idx));

  nrfx_err_t err_code = gpiote_init_button(button_idx, high_accuracy);

  if (err_code == NRFX_SUCCESS)
  {
    m_cb.btns[button_idx].is_used = true;
  }

  return err_code;
}

void btn_debounced_on_press(uint8_t btn_idx, btn_debounced_handler_t handler)
{
  NRFX_ASSERT(btn_is_used(btn_idx));
  NRFX_ASSERT(handler != NULL);

  m_cb.btns[btn_idx].on_press = handler;
}

void btn_debounced_on_release(uint8_t btn_idx, btn_debounced_handler_t handler)
{
  NRFX_ASSERT(btn_is_used(btn_idx));
  NRFX_ASSERT(handler != NULL);

  m_cb.btns[btn_idx].on_release = handler;
}

bool btn_debounced_is_enabled_for(uint8_t button_idx)
{
  return btn_is_used(button_idx);
}
