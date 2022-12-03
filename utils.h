#ifndef __UTILS_H
#define __UTILS_H

#include <limits.h>

#include "nrfx.h"

#define WORD_SIZE 4
#define NULL_ADDR 0

#define IS_WORD_ALIGNED(n)             (n % WORD_SIZE  == 0)
#define LEAST_MULTIPLE_OF_WORD_SIZE(n) (NRFX_CEIL_DIV(n, WORD_SIZE) * WORD_SIZE)

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

#define PARTIALY_IMPLEMENTED_ASSUMING(statement) NRFX_ASSERT(statement)

#define SIZEOF_IN_BITS(v) (sizeof(v) * CHAR_BIT)
#define IS_POWER_OF_2(v)  (v && !(v & (v - 1)))

#endif
