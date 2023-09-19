#ifndef ESTC_EVENT_EMITTER_H__
#define ESTC_EVENT_EMITTER_H__

#include <stddef.h>
#include "app_error.h"

#define EVENT_EMITTER_EVT_CAPACITY    16
#define EVENT_EMITTER_OBS_CAPACITY    16

typedef int event_emitter_evt_t;

typedef void (* event_emitter_cb_t)(); // params are user-specified,
                                       // the user must call it correctly via cb_wrapper

typedef struct event_emitter_obs_s
{
  event_emitter_evt_t   evt;
  uint64_t              target;
  event_emitter_cb_t    cb;
} event_emitter_obs_t;

typedef struct event_emitter_emit_s
{
  event_emitter_evt_t   evt;
  uint64_t              target;
  uint64_t              data;
} event_emitter_emit_t;

typedef void (* event_emitter_cb_wrapper_t)(event_emitter_emit_t * emit,
                                            event_emitter_cb_t     cb);

typedef struct event_emitter_s
{
  event_emitter_obs_t    observers[EVENT_EMITTER_OBS_CAPACITY];
  event_emitter_emit_t   unprocessed_events[EVENT_EMITTER_EVT_CAPACITY];
  size_t                 observers_len;
  size_t                 unprocessed_events_len;
} event_emitter_t;

ret_code_t event_emitter_on(event_emitter_t       * ee,
                            event_emitter_evt_t     evt,
                            uint64_t                target,
                            event_emitter_cb_t      cb);

ret_code_t event_emitter_emit(event_emitter_t     * ee,
                              event_emitter_evt_t   evt,
                              uint64_t              target,
                              uint64_t              data);

size_t event_emitter_process(event_emitter_t            * ee,
                             event_emitter_cb_wrapper_t   cb_wrapper);

#endif

