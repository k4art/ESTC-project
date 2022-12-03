#include "nrf_log.h"

#include "flash/fappdata.h"
#include "utils.h"

#include "fvarstorage.h"
#include "fvarstorage_page_format_defs.h"

#include "fvarstorage_current_page.h"

bool fvarstorage_current_page_is_empty(fvarstorage_t * fvarstorage)
{
  return (fvarstorage->current_value_addr % FAPPDATA_PAGE_SIZE) < PAGE_RECORDS_OFFSET;
}

bool fvarstorage_current_page_is_full(fvarstorage_t * fvarstorage)
{
  size_t page_idx = fappdata_deduce_page_idx_from_addr(fvarstorage->current_value_addr);
  size_t page_addr = FAPPDATA_PAGE_START_ADDR_BY_IDX(page_idx);

  size_t page_format_end = page_addr + PAGE_FORMAT_END_OFFSET;

  NRF_LOG_INFO(
    "[fvarstorage: current_page]\n"
    "\t: current_value_addr = %x\n"
    "\t: page_idx           = %x\n"
    "\t: page_addr          = %x\n"
    "\t: page_formate_end   = %x\n",
    fvarstorage->current_value_addr, page_idx, page_addr, page_format_end);

  return (fvarstorage->current_value_addr + FVARSTORAGE_VALUE_SIZE) >= page_format_end;
}

void fvarstorage_current_page_write_new_record(fvarstorage_t * fvarstorage, const uint32_t * p_value)
{
  NRFX_ASSERT(!fvarstorage_current_page_is_full(fvarstorage));

  fappdata_word_write(fvarstorage->current_value_addr += FVARSTORAGE_VALUE_SIZE, *p_value);
}

void fvarstorage_current_page_read_last_record(fvarstorage_t * fvarstorage, uint32_t * p_value)
{
  NRFX_ASSERT(!fvarstorage_current_page_is_empty(fvarstorage));

  *p_value = fappdata_word_read(fvarstorage->current_value_addr);
}
