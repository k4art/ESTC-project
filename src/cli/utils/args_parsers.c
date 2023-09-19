#include <stdlib.h>

#include "nrf_log.h"

#include "args_parsers.h"

static bool try_parse_string_as_int(char * string, int * result)
{
  const int BASE = 10;

  NRF_LOG_DEBUG("[args_parsers]: %s", NRF_LOG_PUSH(string));

  char * p_parse_end;
  *result = strtol(string, &p_parse_end, BASE);

  return *p_parse_end == '\0';
}

/* Only maxs are dynamic. Mins are taken as zeros. */
cli_args_parser_ret_code_t cli_args_parser_int_series_ranged(char ** args,
                                                             size_t  n_series,
                                                             const int * maxs,
                                                             int * result_ints)
{
  bool is_integer     = true;
  bool is_up_to_max   = true;
  bool is_down_to_min = true;

  for (size_t i = 0; (i < n_series) && is_integer && is_up_to_max && is_down_to_min; i++)
  {
    is_integer = try_parse_string_as_int(args[i], &result_ints[i]);

    is_up_to_max   = result_ints[i] <= maxs[i];
    is_down_to_min = result_ints[i] >= 0;
  }

  if (!is_integer)
  {
    return CLI_PARSER_RET_CODE_INTEGER_EXPECTED;
  }
  else if (!is_up_to_max)
  {
    return CLI_PARSER_RET_CODE_MORE_THAN_MAX;
  }
  else if (!is_down_to_min)
  {
    return CLI_PARSER_RET_CODE_LESS_THAN_MIN;
  }
  else
  {
    return CLI_PARSER_RET_CODE_SUCCESS;
  }
}
