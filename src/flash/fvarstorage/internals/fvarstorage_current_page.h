#ifndef FVARSTORAGE_CURRENT_PAGE_H
#define FVARSTORAGE_CURRENT_PAGE_H

#include "flash/fvarstorage/fvarstorage.h"

bool fvarstorage_current_page_is_empty(fvarstorage_t * fvarstorage);
bool fvarstorage_current_page_is_full(fvarstorage_t * fvarstorage);
void fvarstorage_current_page_write_new_record(fvarstorage_t * fvarstorage, const uint32_t * p_value);
void fvarstorage_current_page_read_last_record(fvarstorage_t * fvarstorage, uint32_t * p_value);

#endif
