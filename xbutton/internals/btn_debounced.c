#include "app_timer.h"

#include "gpio/c_bsp.h"
#include "nrfx_gpiote.h"

#include "nrf_log.h"

#include "btn_debounced.h"

#define NRFX_GPIOTE_CONFIG_IN_SENSE_DEF(var, polarity_in_caps, hi_acc) \
  nrfx_gpiote_in_config_t var = NRFX_GPIOTE_CONFIG_IN_SENSE_ ## polarity_in_caps(hi_acc); \
  var.pull = BUTTON_PULL

typedef enum
{
  BUTTON_PHY_UP,         // normally means button is released
  BUTTON_PHY_DOWN,       // normally means button is pressed
  BUTTON_PHY_BOUNCING,   // normally means button is just started being pressed
} btn_physical_state_t;

typedef enum
{
  BUTTON_ACTION_RELEASE,
  BUTTON_ACTION_PRESS,
  BUTTON_ACTION_DEBOUNCE_PRESSED,
  BUTTON_ACTION_DEBOUNCE_RELEASED,
} btn_action_t;

typedef struct btn_info_s
{
  bool is_used;
  btn_physical_state_t phy_state;
  btn_debounced_state_t debounced_state;
  btn_debounced_handler_t on_press;
  btn_debounced_handler_t on_release;
} btn_info_t;

typedef struct btn_debounced_control_block_s
{
  btn_info_t btns[BUTTONS_NUMBER];
} btn_debounced_control_block_t;

static btn_debounced_control_block_t m_cb;

APP_TIMER_DEF(m_debouncing_timer);

static bool btn_is_used(uint8_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  return m_cb.btns[button_idx].is_used;
}

static void handle_press(uint8_t button_idx)
{
  m_cb.btns[button_idx].debounced_state = BUTTON_PRESSED;

  if (m_cb.btns[button_idx].on_press != NULL)
  {
    m_cb.btns[button_idx].on_press(button_idx, BUTTON_PRESSED);
  }
}

static void handle_release(uint8_t button_idx)
{
  m_cb.btns[button_idx].debounced_state = BUTTON_RELEASED;

  if (m_cb.btns[button_idx].on_release != NULL)
  {
    m_cb.btns[button_idx].on_release(button_idx, BUTTON_RELEASED);
  }
}

static void debouncing_timer_handler(void * context)
{
  uint8_t button_idx = (uint32_t) context;

  if (c_bsp_board_button_state_get(button_idx) == 1)
  {
    // button is still being pressed
    m_cb.btns[button_idx].phy_state = BUTTON_PHY_DOWN;
  }
  else
  {
    m_cb.btns[button_idx].phy_state = BUTTON_PHY_UP;
    handle_release(button_idx);
  }
}

static void debouncing_timer_start(uint8_t button_idx)
{
  void * context = (void *) (uint32_t) button_idx;

  app_timer_start(m_debouncing_timer,
                  APP_TIMER_TICKS(BUTTON_BOUNCING_TIME_MS),
                  context);
}

static void button_to_next_state(uint8_t button_idx, btn_action_t action)
{
  switch (m_cb.btns[button_idx].phy_state) // previous state
  {
    case BUTTON_PHY_UP:
      if (action == BUTTON_ACTION_PRESS)
      {
        NRF_LOG_INFO("[btn_debounced]: [%d] - press", button_idx);

        m_cb.btns[button_idx].phy_state = BUTTON_PHY_BOUNCING;
        handle_press(button_idx);
      }
      else { NRFX_ASSERT(false); }
      break;

    case BUTTON_PHY_DOWN:
      if (action == BUTTON_ACTION_RELEASE)
      {
        NRF_LOG_INFO("[btn_debounced]: [%d] - release", button_idx);

        m_cb.btns[button_idx].phy_state = BUTTON_PHY_UP;
        handle_release(button_idx);
      }
      else { NRFX_ASSERT(false); }
      break;

    case BUTTON_PHY_BOUNCING:
      if (action == BUTTON_ACTION_DEBOUNCE_PRESSED)
      {
        m_cb.btns[button_idx].phy_state = BUTTON_PHY_DOWN;
      }
      else if (action == BUTTON_ACTION_DEBOUNCE_RELEASED)
      {
        m_cb.btns[button_idx].phy_state = BUTTON_PHY_UP;
        handle_release(button_idx);
      }
      else { NRFX_ASSERT(false); }
      break;
  }
}

static void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  uint8_t button_idx = c_bsp_board_pin_to_button_idx(pin);

  NRFX_ASSERT(btn_is_used(button_idx));

  if (c_bsp_board_button_state_get(button_idx) == 1)
  {
    debouncing_timer_start(button_idx);
    button_to_next_state(button_idx, BUTTON_ACTION_PRESS);
  }
  else
  {
    button_to_next_state(button_idx, BUTTON_ACTION_RELEASE);
  }
}

static nrfx_err_t gpiote_init_button(uint8_t button_idx, bool hi_acc)
{
  uint32_t button_pin = c_bsp_board_button_idx_to_pin(button_idx);

  NRFX_GPIOTE_CONFIG_IN_SENSE_DEF(config, TOGGLE, hi_acc);

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
    NRFX_ASSERT(err);
  }

  app_timer_create(&m_debouncing_timer, APP_TIMER_MODE_SINGLE_SHOT, debouncing_timer_handler);
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

  m_cb.btns[btn_idx].on_press = handler;
}

void btn_debounced_on_release(uint8_t btn_idx, btn_debounced_handler_t handler)
{
  NRFX_ASSERT(btn_is_used(btn_idx));

  m_cb.btns[btn_idx].on_release = handler;
}

btn_debounced_state_t btn_debounced_get_state(uint8_t btn_idx)
{
  NRFX_ASSERT(btn_is_used(btn_idx));

  return m_cb.btns[btn_idx].debounced_state;
}

bool btn_debounced_is_enabled_for(uint8_t button_idx)
{
  return btn_is_used(button_idx);
}
