#ifndef FVARSTORAGE_H
#define FVARSTORAGE_H

#include "fvarstorage.h"

void fvarstorage_pages_ensure_format(fvarstorage_t * fvarstorage);
void fvarstorage_pages_format_one(fvarstorage_t * fvarstorage, size_t page_idx);

void fvarstorage_pages_track_current_page_idx(fvarstorage_t * fvarstorage);
size_t fvarstorage_pages_get_current_page_idx(fvarstorage_t * fvarstorage);

void fvarstorage_pages_switch_to_next_page(fvarstorage_t * fvarstorage);

#endif
