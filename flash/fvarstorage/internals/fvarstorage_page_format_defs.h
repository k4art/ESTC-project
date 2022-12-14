#ifndef FVARSTORAGE_PAGEE_FORMAT_DEFS_H
#define FVARSTORAGE_PAGEE_FORMAT_DEFS_H

#include "utils.h"

/*************** PAGE FORMAT ***************/

/*
 *   0        PAGE_HEADER_SIZE         PAGE_SIZE
 *   |--------|------------------------|
 *   | HEADER | RECORDS                |
 *   |--------|------------------------|
 */

/**************** HEADER ****************/

#define MAGIC_NUMBER_SIZE   WORD_SIZE
#define MAGIC_NUMBER_OFFSET 0

#define VARIABLE_ID_SIZE    WORD_SIZE
#define VARIABLE_ID_OFFSET  MAGIC_NUMBER_SIZE

#define MAGIC_NUMBER        0xA8C06D55

#define PAGE_HEADER_SIZE    (VARIABLE_ID_SIZE + MAGIC_NUMBER_SIZE)
#define PAGE_HEADER_OFFSET  0

/* Allow easy memory access by working in words */
STATIC_ASSERT(IS_WORD_ALIGNED(PAGE_HEADER_SIZE));
STATIC_ASSERT(PAGE_HEADER_SIZE <= MAGIC_NUMBER_SIZE + VARIABLE_ID_SIZE);

/*
 *
 *   PAGE_HEADER_OFFSET   MAGIC_NUMBER_SIZE     MAGIC_NUMBER_SIZE + VARIABLE_ID_SIZE
 *   |--------------------|---------------------|
 *   |    MAGIC_NUMBER    |     VARIABLE_ID     |
 *   |--------------------|---------------------|
 *
 *   If the MAGIC_NUMBER or VARIABLE_ID is wrong, the page is considered to be in the zero-day case.
 *
 */

/**************** RECORDS ****************/

#define PAGE_RECORDS_OFFSET PAGE_HEADER_SIZE
#define PAGE_RECORDS_SIZE_MAX (FAPPDATA_PAGE_SIZE - PAGE_HEADER_SIZE)

/*
 * In order to utilize all available space left, use
 *
 * ```
 * #define PAGE_RECORDS_SIZE PAGE_RECORDS_SIZE_MAX
 * ```
 *
 * Another option is to specify a small space to facilitate testing.
 *
 * ```
 * #define PAGE_RECORDS_SIZE (WORD_SIZE * N)
 *
 * STATIC_ASSERT(PAGE_RECORDS_SIZE <= PAGE_RECORDS_SIZE_MAX);
 * ```
 */
#define PAGE_RECORDS_SIZE (WORD_SIZE * 3)

STATIC_ASSERT(PAGE_RECORDS_SIZE <= PAGE_RECORDS_SIZE_MAX);

/*
 * RECORDS:
 *
 * S = PAGE_RECORDS_OFFSET
 * R = size of variable
 * N = the number of records made in the page
 *
 * Assuming S + N*R < PAGE_FORMAT_END_OFFSET:
 *
 *   S              S + R          S + 2R                 S + N*R                             PAGE_FORMAT_END_OFFSET
 *   |--------------|--------------|-------|--------------|--------------|-----|--------------|
 *   |   RECORD_1   |   RECORD_2   |  ...  |   RECORD_N   | *EMPTY_WORD* | ... | *EMPTY_WORD* |
 *   |--------------|--------------|-------|--------------|--------------|-----|--------------|
 *
 * If S + (N + 1)*R >= PAGE_FORMAT_END_OFFSET then there is no more space in the page.
 *
 */

#define PAGE_FORMAT_END_OFFSET (PAGE_RECORDS_OFFSET + PAGE_RECORDS_SIZE)

#endif
