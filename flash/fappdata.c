#include "nrf_log.h"
#include "nrfx_nvmc.h"

#include "fappdata.h"

uint32_t fappdata_word_read(uint32_t addr)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_ADDR(addr));

  return *(uint32_t *) addr;
}

void fappdata_word_write(uint32_t addr, uint32_t word)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_ADDR(addr));

  NRF_LOG_INFO("[fappdata]: word_write(%x, %x)", addr, word);

  nrfx_nvmc_word_write(addr, word);
}

void fappdata_page_erase(uint32_t page_addr)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_PAGE_ADDR(page_addr));

  NRF_LOG_INFO("[fappdata]: page_erase(%x, %x)", page_addr);

  nrfx_nvmc_page_erase(page_addr);
}

void fappdata_wait_until_ready(void)
{
  while (!nrfx_nvmc_write_done_check())
    ;
}

size_t fappdata_deduce_page_idx_from_addr(uint32_t addr)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_ADDR(addr));

  return (FAPPDATA_END_ADDR - addr) / FAPPDATA_PAGE_SIZE;
}
