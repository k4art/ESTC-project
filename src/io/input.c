#include "nrf_log.h"

#include "io/input.h"

static event_emitter_t m_ee;

ret_code_t app_input_emit_(app_input_evt_t evt, uint8_t btn_idx)
{
  return event_emitter_emit(&m_ee, evt, btn_idx, 0 /* no context-related data */);
}

ret_code_t app_input_on_(app_input_evt_t evt, uint8_t btn_idx, app_input_cb_t cb)
{
  return event_emitter_on(&m_ee, evt, btn_idx, (event_emitter_cb_t) cb);
}

static void event_emitter_cb_wrapper(event_emitter_emit_t * emit,
                                     event_emitter_cb_t     cb)
{
  app_input_cb_t event_handler = (app_input_cb_t) cb;

  event_handler(emit->evt, emit->target);
}

size_t app_input_process(void)
{
  return event_emitter_process(&m_ee, event_emitter_cb_wrapper);
}

