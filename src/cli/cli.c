#include <string.h>

#include "nrf_log.h"

#include "utils/strfns.h"

#include "lib/utils.h"
#include "terminal.h"
#include "cli.h"

#define CLI_MAX_COMMAND_NUMBER        10
#define CLI_COMMAND_BUFFER_SIZE       256
#define CLI_COMMAND_MAX_TOKENS_NUMBER 32

typedef struct cli_control_block_s
{
  size_t commands_number;
  char command_buffer[CLI_COMMAND_BUFFER_SIZE];
  const cli_command_desc_t * command_descriptors[CLI_MAX_COMMAND_NUMBER];
} cli_control_block_t;

static cli_control_block_t m_cb;

static const cli_command_desc_t * cli_find_command_by_name(const char * command_name)
{
  for (size_t i = 0; i < m_cb.commands_number; i++)
  {
    if (strcmp(m_cb.command_descriptors[i]->command_name, command_name) == 0)
    {
      return m_cb.command_descriptors[i];
    }
  }

  return NULL;
}

static void help_command_handler(size_t argc, char ** argv);

CLI_COMMAND_DESC_DEF(help_cmd_desc, "help", help_command_handler,
                    "Lists all commands and their descriptions.");

static void help_command_handler(size_t argc, char ** argv)
{
  for (size_t i = 0; i < m_cb.commands_number; i++)
  {
    const char * cmd_name = m_cb.command_descriptors[i]->command_name;
    const char * cmd_desc = m_cb.command_descriptors[i]->desc_text;

    terminal_writef("\r\n\t%s - %s\r\n", cmd_name, cmd_desc);
  }

  terminal_flush();
}

/* Auxiliary handler, does not belong to a command descriptor. */
static void unknown_command_handler(const char * command_name)
{
  // displays three dots if the entered command name was too long
  bool ellipses = strlen(command_name) > 20;

  terminal_writef(" - %.20s%.*s: command not found\r\n", command_name,  ellipses ? 3 : 0, "...");
  terminal_flush();
}

static size_t split_tokens_inplace(char * buffer, size_t buffer_size, char ** tokens, size_t tokens_capacity)
{
  NRFX_ASSERT(buffer_size != 0);

  char * p_char = buffer;
  size_t token_idx = 0;

  skip_whitespaces(&p_char);

  while (*p_char != '\0' && token_idx < tokens_capacity)
  {
    tokens[token_idx++] = p_char;

    skip_non_whitespaces(&p_char);

    if (*p_char == '\0')
    {
      break;
    }

    *p_char = '\0'; // Breaking into tokens "in-place"
                    // (here '\0' is written in place of a space character),
                    // so that tokens[i] will be null-terminated.

    p_char++;

    skip_whitespaces(&p_char);
  }

  return token_idx;
}

static void handle_user_input(void)
{
  static char * argv[CLI_COMMAND_MAX_TOKENS_NUMBER];

  size_t argc = split_tokens_inplace(m_cb.command_buffer,
                                     NRFX_ARRAY_SIZE(m_cb.command_buffer),
                                     argv,
                                     CLI_COMMAND_MAX_TOKENS_NUMBER);

  if (argc == 0)
  {
    NRF_LOG_DEBUG("[cli]: empty command (ignored)");
    return;
  }

  const char * command_name = argv[0];
  const cli_command_desc_t * command_desc = cli_find_command_by_name(command_name);

  NRF_LOG_DEBUG("[cli]: argc = %d", argc);

  if (command_desc == NULL)
  {
    unknown_command_handler(command_name);
  }
  else
  {
    command_desc->handler(argc, argv);
  }
}

void cli_init(void)
{
  cli_register_command(&help_cmd_desc);

  terminal_init(m_cb.command_buffer, CLI_COMMAND_BUFFER_SIZE);
}

void cli_process(void)
{
  terminal_process();

  if (terminal_readline())
  {
    NRF_LOG_DEBUG("[cli]: recieved input");

    handle_user_input();
    terminal_clear_line_buffer();
  }
  else
  {
    // No command is waiting execution
  }
}

void cli_register_command(const cli_command_desc_t * cli_command_desc)
{
  NRFX_ASSERT(m_cb.commands_number < CLI_MAX_COMMAND_NUMBER);

  m_cb.command_descriptors[m_cb.commands_number++] = cli_command_desc;
}
