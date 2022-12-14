#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>

void terminal_init(char * line_buffer, size_t line_buffer_size);
void terminal_process(void);

/*
 * Returns false if no data has been received from user.
 * In this case the buffer does not change.
 *
 * Note: even if true is returned, the string in the buffer can be empty
 *       in the case when user just hit enter.
 */
bool terminal_readline(void);
void terminal_clear_line_buffer(void);

/*
 * Similar to printf. The output will be trucated if it exceeds some extreme value.
 * Call terminal_flush() to make sure all characters are sent.
 */
void terminal_writef(const char * format, ...);

void terminal_flush(void);

#endif
