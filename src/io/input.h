#include "lib/event_emitter.h"
#include "io/xbutton/xbutton.h"

enum
{
  APP_INPUT_CLICK = 1,
  APP_INPUT_DOUBLE_CLICK,
  APP_INPUT_LONG_PRESS_START,
  APP_INPUT_LONG_PRESS_STOP,
};

typedef event_emitter_evt_t app_input_evt_t;

typedef void (* app_input_cb_t)(app_input_evt_t evt, uint8_t btn_idx);

ret_code_t app_input_emit_(app_input_evt_t evt, uint8_t btn_idx);
ret_code_t app_input_on_(app_input_evt_t evt, uint8_t btn_idx, app_input_cb_t cb);

#define app_input_emit(evt, btn_idx)   app_input_emit_(evt, btn_idx)
#define app_input_on(evt, btn_idx, cb) app_input_on_(evt, btn_idx, cb)

size_t app_input_process(void);

