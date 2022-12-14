#include <stdlib.h>

#include "nrf_log.h"

#include "cli/cli.h"
#include "cli/terminal.h"
#include "cli/utils/validators.h"

#include "colors.cli.parsers.h"

#include "../color_picker.h"

#include "color_picker.cli.h"

static void rgb_command_handler(size_t argc, char ** argv)
{
  CLI_VALIDATE_OR_RETURN(argc == 4, "Usage: %s <red> <green> <blue>\r\n", argv[0]);

  user_rgb_color_t user_rgb;

  cli_args_parser_ret_code_t ret = cli_args_parser_user_rgb(&argv[1], &user_rgb);

  switch (ret)
  {
    case CLI_PARSER_RET_CODE_SUCCESS:
    {
      rgb_color_t rgb = rgb_color_from_user_rgb(&user_rgb);
      hsv_color_t hsv = rgb_to_hsv(rgb);

      terminal_writef("Color set to R=%u G=%u B=%u\r\n", user_rgb.red, user_rgb.green, user_rgb.blue);
      color_picker_set_hsv(hsv);

      break;
    }

    case CLI_PARSER_RET_CODE_INTEGER_EXPECTED:
      terminal_writef("Failed. Please, provide only integers.\r\n");
      break;

    case CLI_PARSER_RET_CODE_MORE_THAN_MAX:
      terminal_writef("Failed. All values must be from 0 up to 255.\r\n");
      break;

    case CLI_PARSER_RET_CODE_LESS_THAN_MIN: // below zero
      terminal_writef("Failed. All values must be non-negative.\r\n");
      break;
  }

  terminal_flush();
}

static void hsv_command_handler(size_t argc, char ** argv)
{
  CLI_VALIDATE_OR_RETURN(argc == 4, "Usage: %s <hue> <saturation> <value>\r\n", argv[0]);

  user_hsv_color_t user_hsv;

  cli_args_parser_ret_code_t ret = cli_args_parser_user_hsv(&argv[1], &user_hsv);

  switch (ret)
  {
    case CLI_PARSER_RET_CODE_SUCCESS:
      terminal_writef("Color set to H=%u S=%u V=%u\r\n", user_hsv.hue, user_hsv.saturation, user_hsv.value);
      color_picker_set_hsv(hsv_color_from_user_hsv(&user_hsv));
      break;

    case CLI_PARSER_RET_CODE_INTEGER_EXPECTED:
      terminal_writef("Failed. Please, provide only integers.\r\n");
      break;

    case CLI_PARSER_RET_CODE_MORE_THAN_MAX:
      terminal_writef("Failed. Hue is up to 360, saturation and value are up to 100.\r\n");
      break;

    case CLI_PARSER_RET_CODE_LESS_THAN_MIN: // below zero
      terminal_writef("Failed. All values must be non-negative.\r\n");
      break;
  }

  terminal_flush();
}

CLI_COMMAND_DESC_DEF(rgb_cmd_desc, "rgb", rgb_command_handler, "Sets RGB color");
CLI_COMMAND_DESC_DEF(hsv_cmd_desc, "hsv", hsv_command_handler, "Sets HSV color");

void color_picker_cli_register(void)
{
  cli_register_command(&rgb_cmd_desc);
  cli_register_command(&hsv_cmd_desc);
}
