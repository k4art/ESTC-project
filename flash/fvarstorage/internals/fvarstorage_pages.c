#include "nrf_log.h"

#include "nrfx_nvmc.h"

#include "flash/fappdata.h"

#include "fvarstorage_page_format_defs.h"

#include "fvarstorage_pages.h"

#define CHECK_MASK_BIT_SET(mask, lsb_bit_idx) ((mask & (1 << lsb_bit_idx)) != 0)

static void write_page_header(uint32_t page_addr, uint32_t variable_id)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_PAGE_ADDR(page_addr))

  PARTIALY_IMPLEMENTED_ASSUMING(MAGIC_NUMBER_SIZE == WORD_SIZE);
  PARTIALY_IMPLEMENTED_ASSUMING(VARIABLE_ID_SIZE == WORD_SIZE);

  uint32_t magic_number_addr = page_addr + MAGIC_NUMBER_OFFSET;
  uint32_t variable_id_addr  = page_addr + VARIABLE_ID_OFFSET;

  fappdata_word_write(magic_number_addr, MAGIC_NUMBER);
  fappdata_word_write(variable_id_addr, variable_id);
}

static bool validate_page_header(uint32_t page_addr, uint32_t variable_id)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_PAGE_ADDR(page_addr))

  PARTIALY_IMPLEMENTED_ASSUMING(MAGIC_NUMBER_SIZE == WORD_SIZE);
  PARTIALY_IMPLEMENTED_ASSUMING(VARIABLE_ID_SIZE == WORD_SIZE);

  uint32_t magic_number_addr = page_addr + MAGIC_NUMBER_OFFSET;
  uint32_t variable_id_addr  = page_addr + VARIABLE_ID_OFFSET;

  return fappdata_word_read(magic_number_addr) == MAGIC_NUMBER &&
         fappdata_word_read(variable_id_addr) == variable_id;
}

static bool fvarstorage_pages_validate_all_headers(fvarstorage_t * fvarstorage)
{
  uint32_t pages_mask = fvarstorage->appdata_pages_mask;

  for (int32_t page_idx = 0; page_idx < SIZEOF_IN_BITS(pages_mask); page_idx++)
  {
    if (!CHECK_MASK_BIT_SET(pages_mask, page_idx)) continue;

    uint32_t page_addr = FAPPDATA_PAGE_START_ADDR_BY_IDX(page_idx);

    if (!validate_page_header(page_addr, fvarstorage->variable_id))
    {
      return false;
    }
  }

  return true;
}

static uint32_t find_last_record_addr_in_page(size_t page_idx)
{
  PARTIALY_IMPLEMENTED_ASSUMING(FVARSTORAGE_VALUE_SIZE == WORD_SIZE);

  uint32_t page_start_addr = FAPPDATA_PAGE_START_ADDR_BY_IDX(page_idx);

  uint32_t first_record_addr = page_start_addr + PAGE_RECORDS_OFFSET;
  uint32_t last_record_addr  = first_record_addr + PAGE_RECORDS_SIZE - FVARSTORAGE_VALUE_SIZE;

  for (uint32_t record_addr = last_record_addr; record_addr >= first_record_addr; record_addr -= FVARSTORAGE_VALUE_SIZE)
  {
    if (fappdata_word_read(record_addr) != EMPTY_WORD)
    {
      return record_addr;
    }
  }

  return NULL_ADDR;
}

static uint32_t get_next_page_idx_round_robin(uint32_t pages_mask, size_t from_page_idx)
{
  NRFX_ASSERT(CHECK_MASK_BIT_SET(pages_mask, from_page_idx));

  for (int32_t page_idx = from_page_idx + 1; page_idx < SIZEOF_IN_BITS(pages_mask); page_idx++)
  {
    if (CHECK_MASK_BIT_SET(pages_mask, page_idx))
    {
      return page_idx;
    }
  }

  for (int32_t page_idx = 0; page_idx < from_page_idx; page_idx++)
  {
    if (CHECK_MASK_BIT_SET(pages_mask, page_idx))
    {
      return page_idx;
    }
  }

  return from_page_idx;
}

static uint32_t get_first_record_addr_in_page(size_t page_idx)
{
  size_t page_addr = FAPPDATA_PAGE_START_ADDR_BY_IDX(page_idx);

  return page_addr + PAGE_RECORDS_OFFSET;
}

static uint32_t fvarstorage_pages_zero_day_addr_get(fvarstorage_t * fvarstorage)
{
  NRFX_ASSERT(fvarstorage->appdata_pages_mask != 0);

  size_t page_idx = __builtin_ctz(fvarstorage->appdata_pages_mask);
  uint32_t first_record_addr = get_first_record_addr_in_page(page_idx);

  // this keeps invariant: next_value_addr = current_value_addr + FVARSTORAGE_VALUE_SIZE
  return first_record_addr - FVARSTORAGE_VALUE_SIZE;
}

void fvarstorage_pages_format_one(fvarstorage_t * fvarstorage, size_t page_idx)
{
  uint32_t page_addr = FAPPDATA_PAGE_START_ADDR_BY_IDX(page_idx);

  NRF_LOG_INFO("[fvarstorage]: erasing page at %x", page_addr);

  nrfx_nvmc_page_erase(page_addr);
  write_page_header(page_addr, fvarstorage->variable_id);
}

void fvarstorage_pages_ensure_format(fvarstorage_t * fvarstorage)
{
  uint32_t pages_mask = fvarstorage->appdata_pages_mask;

  for (int32_t page_idx = 0; page_idx < SIZEOF_IN_BITS(pages_mask); page_idx++)
  {
    if ((pages_mask & (1 << page_idx)) == 0) continue;

    uint32_t page_addr = FAPPDATA_PAGE_START_ADDR_BY_IDX(page_idx);

    if (!validate_page_header(page_addr, fvarstorage->variable_id))
    {
      fvarstorage_pages_format_one(fvarstorage, page_idx);
    }
  }

  NRFX_ASSERT(fvarstorage_pages_validate_all_headers(fvarstorage));
}

void fvarstorage_pages_track_current_page_idx(fvarstorage_t * fvarstorage)
{
  NRFX_ASSERT(fvarstorage_pages_validate_all_headers(fvarstorage));

  uint32_t pages_mask = fvarstorage->appdata_pages_mask;

  for (int32_t page_idx = 0; page_idx < SIZEOF_IN_BITS(pages_mask); page_idx++)
  {
    if (!CHECK_MASK_BIT_SET(pages_mask, page_idx)) continue;

    uint32_t last_record_addr = find_last_record_addr_in_page(page_idx);

    if (last_record_addr != NULL_ADDR)
    {
      fvarstorage->current_value_addr = last_record_addr;
      return;
    }
  }

  fvarstorage->current_value_addr = fvarstorage_pages_zero_day_addr_get(fvarstorage);
}

size_t fvarstorage_pages_get_current_page_idx(fvarstorage_t * fvarstorage)
{
  NRFX_ASSERT(fvarstorage_pages_validate_all_headers(fvarstorage));
  NRFX_ASSERT(fvarstorage->current_value_addr != NULL_ADDR);

  return fappdata_deduce_page_idx_from_addr(fvarstorage->current_value_addr);
}

void fvarstorage_pages_switch_to_next_page(fvarstorage_t * fvarstorage)
{
  NRFX_ASSERT(fvarstorage_pages_validate_all_headers(fvarstorage));

  uint32_t page_idx = fappdata_deduce_page_idx_from_addr(fvarstorage->current_value_addr);
  uint32_t next_page_idx = get_next_page_idx_round_robin(fvarstorage->appdata_pages_mask, page_idx);

  fvarstorage->current_value_addr = get_first_record_addr_in_page(next_page_idx) - FVARSTORAGE_VALUE_SIZE;
}
