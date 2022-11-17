#ifndef __UTILS_H
#define __UTILS_H

#define CALL_IF_NOT_NULL(fn, ...) \
  if (fn != NULL) fn(__VA_ARGS__)

#endif
