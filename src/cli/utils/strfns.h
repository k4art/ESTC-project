#ifndef STRFNS_H
#define STRFNS_H

/*
 * After execution *buffer will point to the first occured non-whitespace character.
 *
 * Note: if buffer contains only whitespace characters, *buffer will point to '\0'.
 */
void skip_whitespaces(char ** buffer);

/*
 * After execution *buffer will point to the first occured whitespace character.
 *
 * Note: if buffer contains only non-whitespace characters, buffer will point to '\0'.
 */
void skip_non_whitespaces(char ** buffer);

#endif
