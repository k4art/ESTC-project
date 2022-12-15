#include <ctype.h>

#include "strfns.h"

void skip_whitespaces(char ** buffer)
{
  char * p_char = *buffer;

  while (isspace((unsigned char) *p_char))
  {
    p_char++;
  }

  *buffer = p_char;
}

void skip_non_whitespaces(char ** buffer)
{
  char * p_char = *buffer;

  while (*p_char != '\0' && !isspace((unsigned char) *p_char))
  {
    p_char++;
  }

  *buffer = p_char;
}
