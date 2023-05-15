#ifndef FVARSTORAGE_H__
#define FVARSTORAGE_H__

#include <stdint.h>

/*
 * Utilization of the pages happens in the order of increasing indexes (the lowest first).
*
 * The current implementation supports only word-sized values and reserves 0xFFFFFFFF value for internal purposes,
 * so it can not save this value.
 */

#define FVARSTORAGE_MIN_PAGES_NUMBER 2

#define EMPTY_WORD    0xFFFFFFFF
#define RESERVED_WORD EMPTY_WORD

#define FVARSTORAGE_VALUE_SIZE 4

typedef struct fvarstorage_s
{
  uint32_t current_value_addr;
  uint32_t variable_id;
  uint32_t appdata_pages_mask;
} fvarstorage_t;

void fvarstorage_init(fvarstorage_t * fvarstorage, uint32_t variable_id, uint32_t pages_mask);

void fvarstorage_save(fvarstorage_t * fvarstorage, const uint32_t * p_value);

/*
 * If no value was found, does not change the value and returns false.
 */
bool fvarstorage_restore(fvarstorage_t * fvarstorage, uint32_t * p_value);

/*
 * fvarstorage does not allow all possible 32-bit patterns be saved.
 */
bool fvarstorage_can_be_saved(const uint32_t * p_value);

#endif
