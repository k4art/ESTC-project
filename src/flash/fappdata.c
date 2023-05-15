#include "nrf_fstorage.h"
#include "nrf_log.h"
#include "nrf_log_backend_usb.h"
#include "nrf_log_ctrl.h"
#include "nrf_soc.h"
#include "nrf_fstorage_sd.h"

#include "lib/utils.h"

#include "fappdata.h"

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
  .evt_handler = fstorage_evt_handler,
  .start_addr  = FAPPDATA_START_ADDR,
  .end_addr    = FAPPDATA_END_ADDR,
};

static void power_manage(void)
{
  (void) sd_app_evt_wait();
  __WFE();
}

static void wait_for_flash_ready(void)
{
  while (nrf_fstorage_is_busy(&fstorage))
  {
    power_manage();
  }
}

void fappdata_init(void)
{
  CHECKED(nrf_fstorage_init(&fstorage, &nrf_fstorage_sd, NULL));
}

uint32_t fappdata_word_read(uint32_t addr)
{
  uint32_t result;

  NRFX_ASSERT(FAPPDATA_IS_VALID_ADDR(addr));

  CHECKED(nrf_fstorage_read(&fstorage, addr, &result, sizeof(result)));
  wait_for_flash_ready();

  return result;
}

void fappdata_word_write(uint32_t addr, uint32_t word)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_ADDR(addr));

  NRF_LOG_INFO("[fappdata]: word_write(%x, %x)", addr, word);
  CHECKED(nrf_fstorage_write(&fstorage, addr, &word, sizeof(word), NULL));
  wait_for_flash_ready();
}

void fappdata_page_erase(uint32_t page_addr)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_PAGE_ADDR(page_addr));

  NRF_LOG_INFO("[fappdata]: page_erase(%x, %x)", page_addr);

  CHECKED(nrf_fstorage_erase(&fstorage, page_addr, 1, NULL));
  wait_for_flash_ready();
}

size_t fappdata_deduce_page_idx_from_addr(uint32_t addr)
{
  NRFX_ASSERT(FAPPDATA_IS_VALID_ADDR(addr));

  return (FAPPDATA_END_ADDR - addr) / FAPPDATA_PAGE_SIZE;
}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
  if (p_evt->result != NRF_SUCCESS)
  {
    NRF_LOG_INFO(
        "--> Event received: ERROR while executing an fstorage operation.");
    return;
  }

  switch (p_evt->id) {
  case NRF_FSTORAGE_EVT_WRITE_RESULT:
  {
    NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                 p_evt->len, p_evt->addr);
  } break;

  case NRF_FSTORAGE_EVT_ERASE_RESULT:
  {
    NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                 p_evt->len, p_evt->addr);
  } break;

  default:
    break;
  }
}
