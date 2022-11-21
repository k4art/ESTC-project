#ifndef __UTILS_H
#define __UTILS_H

#define CALL_IF_NOT_NULL(fn, ...)    \
  do                                 \
  {                                  \
    if (fn != NULL) fn(__VA_ARGS__); \
  } while (0)                        \

#endif
