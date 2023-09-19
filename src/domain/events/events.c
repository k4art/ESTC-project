#include "lib/event_emitter.h"

#include "events.h"

static event_emitter_t m_app_events;

ret_code_t app_events_on_(app_events_evt_t evt, app_events_cb_t cb)
{
	return event_emitter_on(&m_app_events, evt, 0, cb);
}

ret_code_t app_events_emit_(app_events_evt_t evt, uint32_t data)
{
	return event_emitter_emit(&m_app_events, evt, 0, data);
}

static void cb_wrapper(event_emitter_emit_t * emit, event_emitter_cb_t cb)
{
	app_events_cb_t callback = (app_events_cb_t) cb;

	callback(emit->evt, emit->data);
}

size_t app_events_process(void)
{
	return event_emitter_process(&m_app_events, cb_wrapper);
}

