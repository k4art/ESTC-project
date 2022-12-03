#ifndef __UTILS_H
#define __UTILS_H

#include "nrfx.h"

#define CALL_IF_NOT_NULL(fn, ...)    \
  do                                 \
  {                                  \
    if (fn != NULL) fn(__VA_ARGS__); \
  } while (0)                        \

#define UNREACHABLE_RETURN(return_value) \
  do {                                   \
    NRFX_ASSERT(0);                      \
    return return_value;                 \
  } while (0)                            \

#endif
