#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "cli/terminal.h"

#define CLI_VALIDATE_OR_RETURN(condition, format, ...) \
  do                                                   \
  {                                                    \
    if (!(condition))                                  \
    {                                                  \
      terminal_writef(format, __VA_ARGS__);            \
      terminal_flush();                                \
      return;                                          \
    }                                                  \
  } while (0);                                         \

#endif
