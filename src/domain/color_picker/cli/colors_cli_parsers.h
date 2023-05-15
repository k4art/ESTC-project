#ifndef COLORS_CLI_PARSERS_H
#define COLORS_CLI_PARSERS_H

#include "user_input_types.h"
#include "cli/utils/args_parsers.h"

cli_args_parser_ret_code_t cli_args_parser_user_rgb(char ** args, user_rgb_color_t * p_user_rgb);
cli_args_parser_ret_code_t cli_args_parser_user_hsv(char ** args, user_hsv_color_t * p_user_hsv);

#endif
