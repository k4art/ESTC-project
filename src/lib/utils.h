#ifndef __UTILS_H
#define __UTILS_H

#include <limits.h>

#include "nrf_log.h"
#include "nrf_log_backend_usb.h"
#include "nrf_log_ctrl.h"
#include "nrf_strerror.h"
#include "nrfx.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"

#include "lib/gpio/c_bsp.h"

#define WORD_SIZE     4
#define NULL_ADDR     0
#define INVALID_CHAR -1

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

#define CHECKED(expr) do          \
{                                 \
  ret_code_t ret = (expr);        \
  if (ret != 0) debug_fault(__FILE__, __LINE__, ret); \
} while (0)

#define BYTE_AT(bytes, n) ((bytes >> (n * 8)) & 0xFF)

#define UUID_BIGEND(a4, b2, c2, d2, e6)   \
 { BYTE_AT(0x ## e6, 0), \
   BYTE_AT(0x ## e6, 1), \
   BYTE_AT(0x ## e6, 2), \
   BYTE_AT(0x ## e6, 3), \
   BYTE_AT(0x ## e6, 4), \
   BYTE_AT(0x ## e6, 5), \
                   \
   BYTE_AT(0x ## d2, 0), \
   BYTE_AT(0x ## d2, 1), \
                   \
   BYTE_AT(0x ## c2, 0), \
   BYTE_AT(0x ## c2, 1), \
                   \
   BYTE_AT(0x ## b2, 0), \
   BYTE_AT(0x ## b2, 1), \
                   \
   BYTE_AT(0x ## a4, 0), \
   BYTE_AT(0x ## a4, 1), \
   BYTE_AT(0x ## a4, 2), \
   BYTE_AT(0x ## a4, 3), \
 }

static inline void debug_fault(const char * file, int line, ret_code_t ret)
{
  nrfx_systick_state_t systick;
  
  nrfx_systick_init();
  nrfx_systick_get(&systick);

  while (true) 
  {
    if (nrfx_systick_test(&systick, 250000))
    {
      nrfx_systick_get(&systick);

      NRF_LOG_INFO("![ERROR]! %s:%d: %s", file, line, nrf_strerror_get(ret));
      c_bsp_board_led_invert(0);
    }
    
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
  }
}

#endif
