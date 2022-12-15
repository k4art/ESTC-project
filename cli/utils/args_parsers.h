#ifndef ARGV_PARSERS_H
#define ARGV_PARSERS_H

typedef enum cli_args_parser_ret_code_e
{
  CLI_PARSER_RET_CODE_SUCCESS = 0,
  CLI_PARSER_RET_CODE_INTEGER_EXPECTED,
  CLI_PARSER_RET_CODE_MORE_THAN_MAX,
  CLI_PARSER_RET_CODE_LESS_THAN_MIN,
} cli_args_parser_ret_code_t;

cli_args_parser_ret_code_t cli_args_parser_int_series_ranged(char ** args,
                                                             size_t  n_series,
                                                             const int * maxs,
                                                             int * result_ints);

#endif
