#ifndef ESTC_DOMAIN_EVENTS_H__
#define ESTC_DOMAIN_EVENTS_H__

#include "nrf_log.h"

#include "lib/event_emitter.h"
#include "lib/utils.h"

enum
{
  APP_EVENTS_EVT_LED_SYNC = 1,
  APP_EVENTS_EVT_COLOR_PICKER_SYNC,
};

typedef event_emitter_evt_t app_events_evt_t;

typedef void (* app_events_cb_t)(app_events_evt_t evt, uint32_t data);

ret_code_t app_events_on_(app_events_evt_t evt, app_events_cb_t cb);
ret_code_t app_events_emit_(app_events_evt_t evt, uint32_t data);
size_t     app_events_process(void);

#define app_events_on(evt, cb)       app_events_on_(evt, cb)
#define app_events_emit(evt, ctx)    app_events_emit_(evt, ctx)

#endif
