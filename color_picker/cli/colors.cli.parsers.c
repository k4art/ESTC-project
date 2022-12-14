#include "user_input_types.h"
#include "cli/utils/args_parsers.h"

#include "colors.cli.parsers.h"

cli_args_parser_ret_code_t cli_args_parser_user_rgb(char ** args, user_rgb_color_t * p_user_rgb)
{
  cli_args_parser_ret_code_t ret;

  ret = cli_args_parser_int_series_ranged(args,
                                          RGB_COMPONENTS_NUMBER,
                                          USER_RGB_COLOR_MAXS,
                                          p_user_rgb->components);

  return ret;
}

cli_args_parser_ret_code_t cli_args_parser_user_hsv(char ** args, user_hsv_color_t * p_hsv_color)
{
  cli_args_parser_ret_code_t ret;

  ret = cli_args_parser_int_series_ranged(args,
                                          HSV_COMPONENTS_NUMBER,
                                          USER_HSV_COLOR_MAXS,
                                          p_hsv_color->components);

  return ret;
}
