#ifndef CLI_H
#define CLI_H

#include <stddef.h>

typedef void (* cli_command_handler_t)(size_t argc, char ** argv);

/*
 * Should not be used directly, see CLI_COMMAND_DESC_DEF.
 */
typedef struct cli_command_desc_s
{
  const char * command_name;
  const char * desc_text;
  cli_command_handler_t handler;
} cli_command_desc_t;

#define CLI_COMMAND_DESC_DEF(var, name, cmd_handler, description) \
  static cli_command_desc_t var =                                 \
  {                                                               \
    .command_name = name,                                         \
    .desc_text = description,                                     \
    .handler = cmd_handler,                                       \
  }

void cli_init(void);

void cli_process(void);

/*
 * Should be called with a cli_command_desc_t declared using CLI_COMMAND_DESC_DEF.
 */
void cli_register_command(const cli_command_desc_t * cli_command_desc);

#endif
