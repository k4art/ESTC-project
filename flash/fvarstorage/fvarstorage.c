#include "nrf_log.h"

#include "flash/fappdata.h"

#include "internals/fvarstorage_page_format_defs.h"
#include "internals/fvarstorage_pages.h"
#include "internals/fvarstorage_current_page.h"

#include "fvarstorage.h"

#define PAGES_MASK_WIDTH 32
#define VALUE_SIZE       4
#define NULL_ADDR        0

#define MASK_ONLY_ONE_BIT_SET(n) IS_POWER_OF_2(n)

/* In the current implementation only word-sized values are supported. */
STATIC_ASSERT(VALUE_SIZE == WORD_SIZE);

void fvarstorage_init(fvarstorage_t * fvarstorage, uint32_t variable_id, uint32_t pages_mask)
{
  NRFX_ASSERT(pages_mask != 0);
  NRFX_ASSERT(!MASK_ONLY_ONE_BIT_SET(pages_mask));

  fvarstorage->appdata_pages_mask = pages_mask;
  fvarstorage->variable_id        = variable_id;

  fvarstorage_pages_ensure_format(fvarstorage);
  fvarstorage_pages_track_current_page_idx(fvarstorage);
}

void fvarstorage_save(fvarstorage_t * fvarstorage, const uint32_t * p_value)
{
  NRFX_ASSERT(fvarstorage_can_be_saved(p_value));

  if (fvarstorage_current_page_is_full(fvarstorage))
  {
    size_t full_page_idx = fvarstorage_pages_get_current_page_idx(fvarstorage);

    fvarstorage_pages_switch_to_next_page(fvarstorage);
    fvarstorage_current_page_write_new_record(fvarstorage, p_value);

    fvarstorage_pages_format_one(fvarstorage, full_page_idx);
  }
  else
  {
    fvarstorage_current_page_write_new_record(fvarstorage, p_value);
  }
}

bool fvarstorage_restore(fvarstorage_t * fvarstorage, uint32_t * p_value)
{
  if (fvarstorage_current_page_is_empty(fvarstorage))
  {
    return false;
  }
  else
  {
    fvarstorage_current_page_read_last_record(fvarstorage, p_value);
    return true;
  }
}

bool fvarstorage_can_be_saved(const uint32_t * p_value)
{
  return *p_value != RESERVED_WORD;
}
