#include "event_emitter.h"
#include "nrf_error.h"

ret_code_t event_emitter_on(event_emitter_t     * ee,
                            event_emitter_evt_t   evt,
                            uint64_t              target,
                            event_emitter_cb_t    cb)
{
  if (ee->observers_len >= EVENT_EMITTER_OBS_CAPACITY)
  {
    return NRF_ERROR_NO_MEM;
  }

  ee->observers[ee->observers_len++] = (event_emitter_obs_t)
  {
    .evt    = evt,
    .target = target,
    .cb     = cb,
  };

  return NRF_SUCCESS;
}

ret_code_t event_emitter_emit(event_emitter_t     * ee,
                              event_emitter_evt_t   evt,
                              uint64_t              target,
                              uint64_t              data)
{
  if (ee->unprocessed_events_len >= EVENT_EMITTER_OBS_CAPACITY)
  {
    return NRF_ERROR_NO_MEM;
  }

  ee->unprocessed_events[ee->unprocessed_events_len++] = (event_emitter_emit_t)
  {
    .evt    = evt,
    .data   = data,
    .target = target,
  };

  return NRF_SUCCESS;
}

static size_t dispatch_event(event_emitter_t            * ee,
                             event_emitter_emit_t       * emit,
                             event_emitter_cb_wrapper_t   cb_wrapper)
{
  size_t count = 0;
  
  for (size_t i = 0; i < ee->observers_len; i++)
  {
    event_emitter_obs_t * obs = &ee->observers[i];

    if (obs->evt == emit->evt && obs->target == emit->target)
    {
      cb_wrapper(emit, obs->cb);
      count++;
    }
  }

  return count;
}

size_t event_emitter_process(event_emitter_t            * ee,
                             event_emitter_cb_wrapper_t   cb_wrapper)
{
  size_t count = 0;
  event_emitter_emit_t * emit;

  for (size_t i = 0; i < ee->unprocessed_events_len; i++)
  {
    emit = &ee->unprocessed_events[i];
    count += dispatch_event(ee, emit, cb_wrapper);
  }

  ee->unprocessed_events_len = 0;
  return count;
}

