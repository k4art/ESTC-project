#include "xbutton.h"

#include "nrfx_gpiote.h"
#include "gpio/c_bsp.h"

#define NRFX_GPIOTE_CONFIG_IN_SENSE_DEF(var, polarity_in_caps, hi_acc) \
  nrfx_gpiote_in_config_t var = NRFX_GPIOTE_CONFIG_IN_SENSE_ ## polarity_in_caps(hi_acc); \
  var.pull = BUTTON_PULL

typedef struct btn_info_s
{
  bool is_used;

  xbutton_handler_t on_click;
} btn_info_t;

typedef struct
{
  btn_info_t btns[BUTTONS_NUMBER];
} xbutton_control_block_t;

xbutton_control_block_t m_cb;

static bool btn_is_used(uint8_t button_idx)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  return m_cb.btns[button_idx].is_used == false;
}

static void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  (void) action;

  uint8_t button_idx = c_bsp_board_pin_to_button_idx(pin);

  // currently dummy implementation considering unpressing as click
  if (c_bsp_board_button_state_get(button_idx) == 0)
  {
    m_cb.btns[button_idx].on_click(pin);
  }
}

static nrfx_err_t gpiote_init_button(uint8_t button_idx, bool hi_acc)
{
  uint32_t button_pin = c_bsp_board_button_idx_to_pin(button_idx);

  NRFX_GPIOTE_CONFIG_IN_SENSE_DEF(config, TOGGLE, hi_acc);

  nrfx_err_t err_code = nrfx_gpiote_in_init(button_pin, &config, gpiote_event_handler);

  if (err_code == NRFX_SUCCESS)
  {
    m_cb.btns[button_idx].is_used = true;
  }

  nrfx_gpiote_in_event_enable(button_pin, true);

  return err_code;
}

nrfx_err_t xbutton_init(void)
{
  if (!nrfx_gpiote_is_init())
  {
    return nrfx_gpiote_init();
  }
  else
  {
    return NRFX_SUCCESS;
  }
}

nrfx_err_t xbutton_enable(uint8_t button_idx, bool high_accuracy)
{
  NRFX_ASSERT(!btn_is_used(button_idx));

  return gpiote_init_button(button_idx, high_accuracy);
}

void xbutton_on_click(uint8_t button_idx, xbutton_handler_t handler)
{
  NRFX_ASSERT(IS_VALID_BUTTON_IDX(button_idx));

  m_cb.btns[button_idx].on_click = handler;
}
