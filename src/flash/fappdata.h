#ifndef FAPPDATA_DEFS_H__
#define FAPPDATA_DEFS_H__

#include "sdk_config.h"
#include "lib/utils.h"

#define BOOTLOADER_START_ADDR 0xE0000

#define FAPPDATA_PAGE_SIZE    0x1000
#define FAPPDATA_PAGES_NUMBER 2

// BOOTLOADER_START_ADDR - FDS (Peer Manager)
#define FAPPDATA_END_ADDR     (  BOOTLOADER_START_ADDR                            \
                                - FDS_VIRTUAL_PAGE_SIZE * 4 * FDS_VIRTUAL_PAGES)

#define FAPPDATA_START_ADDR   (FAPPDATA_END_ADDR - FAPPDATA_PAGE_SIZE * FAPPDATA_PAGES_NUMBER)

/* Should not be used if no pages for Application Data are avaiable. */
STATIC_ASSERT(FAPPDATA_PAGES_NUMBER >= 1);

/* This assures the page-number-to-page-address convertation works correctly. */
STATIC_ASSERT(FAPPDATA_START_ADDR % FAPPDATA_PAGE_SIZE == 0);
STATIC_ASSERT(FAPPDATA_END_ADDR % FAPPDATA_PAGE_SIZE == 0);

/*
 * Counting of application data pages proceeds from higher addresses to lowers (END_ADDR to START_ADDR)
 * so that if NRF_DFU_DATA_AREA_SIZE increases, the numeration will not be affected.
 */

#define FAPPDATA_PAGE_MASK_BY_IDX(n)         (1 << n)

#define FAPPDATA_PAGE_START_ADDR_BY_IDX(idx) (FAPPDATA_END_ADDR - (idx + 1) * FAPPDATA_PAGE_SIZE)
#define FAPPDATA_PAGE_END_ADDR_BY_IDX(idx)   (FAPPDATA_PAGE_START_ADDR_BY_IDX(idx) + FAPPDATA_PAGE_SIZE)

/* The last page starts at the beginning of Application Data section. */
STATIC_ASSERT(FAPPDATA_PAGE_START_ADDR_BY_IDX(FAPPDATA_PAGES_NUMBER - 1) == FAPPDATA_START_ADDR);

/* The first page ends at the end of Application Data section. */
STATIC_ASSERT(FAPPDATA_PAGE_END_ADDR_BY_IDX(0) == FAPPDATA_END_ADDR);

#define FAPPDATA_IS_VALID_ADDR(addr) \
   (addr >= FAPPDATA_START_ADDR && addr < FAPPDATA_END_ADDR)

#define FAPPDATA_IS_VALID_PAGE_IDX(page_idx) (page_idx >= 0 && page_idx < FAPPDATA_PAGES_NUMBER)
#define FAPPDATA_IS_VALID_PAGE_ADDR(page_addr) \
   (page_addr >= FAPPDATA_START_ADDR &&        \
    page_addr < FAPPDATA_END_ADDR &&           \
    page_addr % FAPPDATA_PAGE_SIZE == 0)

void fappdata_init(void);
uint32_t fappdata_word_read(uint32_t addr);
void fappdata_word_write(uint32_t addr, uint32_t word);
void fappdata_page_erase(uint32_t page_addr);

size_t fappdata_deduce_page_idx_from_addr(uint32_t addr);

#endif
