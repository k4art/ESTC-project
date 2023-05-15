#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "nrf_log.h"

#include "lib/utils.h"
#include "internals/usb_connection.h"
#include "terminal.h"

#define TERMINAL_WRITE_BUFFER_SIZE 255

#define TERMINAL_PROMPT "> "

#define ASCII_DEL 0x7F
#define ASCII_FF  0x0C // ^L

#define ASCII_SEQ_CLEAR_SCREEN              "\033[2J"
#define ASCII_SEQ_CURSOR_TO_TOP_LEFT_CORNER "\033[;H"

typedef struct terminal_control_block_s
{
  char * line_buffer;
  size_t line_buffer_size;
  size_t line_buffer_used_bytes;

  int input_char;
} terminal_control_block_t;

static terminal_control_block_t m_cb =
{
  .input_char = INVALID_CHAR,
};

static bool line_buffer_is_empty(void)
{
  return m_cb.line_buffer_used_bytes == 0;
}

static bool line_buffer_is_full(void)
{
  /* One byte is left to \0-terminator */
  NRFX_ASSERT(m_cb.line_buffer_used_bytes < m_cb.line_buffer_size);

  return m_cb.line_buffer_used_bytes + 1 == m_cb.line_buffer_size;
}

static void line_buffer_null_terminate(void)
{
  NRFX_ASSERT(m_cb.line_buffer_used_bytes <= m_cb.line_buffer_size);

  m_cb.line_buffer[m_cb.line_buffer_used_bytes] = '\0';
}

static void line_buffer_pop(void)
{
  NRFX_ASSERT(!line_buffer_is_empty());

  m_cb.line_buffer_used_bytes--;
}

static void line_buffer_push(char ch)
{
  NRFX_ASSERT(!line_buffer_is_full());

  m_cb.line_buffer[m_cb.line_buffer_used_bytes++] = ch;
}

static bool is_line_ending(char ch)
{
  return ch == '\r' || ch == '\n';
}

static void terminal_prompt_display_same_line(void)
{
  terminal_writef("> ");
  terminal_flush();
}

static void terminal_prompt_display(void)
{
  terminal_writef("\r\n");
  terminal_prompt_display_same_line();
}

void terminal_init(char * line_buffer, size_t line_buffer_size)
{
  m_cb.line_buffer      = line_buffer;
  m_cb.line_buffer_size = line_buffer_size;

  usb_connection_init();
}

void terminal_process(void)
{
  usb_connection_process();
}

bool terminal_readline(void)
{
  /* If there was a line, it should have been processed and cleared. */
  NRFX_ASSERT(!is_line_ending(m_cb.input_char) || line_buffer_is_empty());

  if (!usb_connection_port_is_open())
  {
    return false;
  }

  if (is_line_ending(m_cb.input_char))
  {
    terminal_prompt_display();
  }

  while ((m_cb.input_char = usb_connection_read_char()) != INVALID_CHAR)
  {
    NRF_LOG_DEBUG("[terminal]: line buffer bytes used = %d", m_cb.line_buffer_used_bytes);
    NRF_LOG_DEBUG("[terminal]: new byte read [%02x]", m_cb.input_char);

    if (m_cb.input_char == '\r' || m_cb.input_char == '\n')
    {
      /* The input line buffer does not contain the line-ending characters */
      /* The rest characters are left in the usb_connection internal buffer */

      line_buffer_null_terminate();

      terminal_writef("\r\n");
      terminal_flush();

      return true;
    }

    if (m_cb.input_char == ASCII_DEL)
    {
      if (!line_buffer_is_empty())
      {
        line_buffer_pop();

        terminal_writef("\b\x20\b");
        terminal_flush();
      }

      return false;
    }

    if (m_cb.input_char == ASCII_FF)
    {
      terminal_writef(ASCII_SEQ_CLEAR_SCREEN ""
                      ASCII_SEQ_CURSOR_TO_TOP_LEFT_CORNER ""
                      TERMINAL_PROMPT);

      terminal_writef("%.*s", m_cb.line_buffer_used_bytes, m_cb.line_buffer);
      
      terminal_flush();

      return false;
    }

    if (line_buffer_is_full())
    {
      /* The character is skipt without being echoed */
      continue;
    }

    terminal_writef("%c", m_cb.input_char);
    terminal_flush();

    line_buffer_push(m_cb.input_char);
  }

  /* The characters received, if any, were processes in the while loop */
  return false;
}

void terminal_clear_line_buffer(void)
{
  m_cb.line_buffer_used_bytes = 0;
}

void terminal_writef(const char * format, ...)
{
  static char write_buffer[TERMINAL_WRITE_BUFFER_SIZE];

  va_list args;
  va_start(args, format);

  size_t length = vsnprintf(write_buffer, TERMINAL_WRITE_BUFFER_SIZE, format, args);

  va_end(args);

  NRF_LOG_DEBUG("[terminal]: writef length = %u", length);

  usb_connection_write(write_buffer, length);
}

void terminal_flush(void)
{
  usb_connection_flush();
}
