#include "nrf_log.h"
#include "nrf_queue.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

#include "utils.h"

#include "usb_connection.h"

#define USB_CONNECTION_RX_QUEUE_SIZE 64
#define USB_CONNECTION_TX_QUEUE_SIZE 64

#define USB_CONNECTION_READ_SIZE 1

static bool m_is_port_open;

static volatile bool m_is_tx_pending;
// static volatile bool m_is_rx_done;

NRF_QUEUE_DEF(char, m_rx_queue, USB_CONNECTION_RX_QUEUE_SIZE, NRF_QUEUE_MODE_NO_OVERFLOW);
NRF_QUEUE_DEF(char, m_tx_queue, USB_CONNECTION_TX_QUEUE_SIZE, NRF_QUEUE_MODE_NO_OVERFLOW);

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CLI_USB_CDC_ACM_COMM_INTERFACE,
                            CLI_USB_CDC_ACM_DATA_INTERFACE,
                            CLI_USB_CDC_ACM_COMM_EPIN,
                            CLI_USB_CDC_ACM_DATA_EPIN,
                            CLI_USB_CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
  static char rx_buffer[USB_CONNECTION_READ_SIZE];

  switch (event)
  {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
      m_is_port_open = true;

      /* Initiate reading */
      ret_code_t ret = app_usbd_cdc_acm_read(&usb_cdc_acm, rx_buffer, USB_CONNECTION_READ_SIZE);

      NRFX_ASSERT(ret == NRF_ERROR_IO_PENDING);
      break;
    }

    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
      m_is_port_open = false;
      break;
    }

    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
      NRF_LOG_DEBUG("[usb_connection]: tx done");

      m_is_tx_pending = false;
      break;
    }

    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
      ret_code_t ret;

      do
      {
        NRF_LOG_DEBUG("[usb_connection]: rx done");

        size_t n_added = nrf_queue_in(&m_rx_queue, rx_buffer, NRFX_ARRAY_SIZE(rx_buffer));

        if (n_added < NRFX_ARRAY_SIZE(rx_buffer))
        {
          NRF_LOG_WARNING("[usb_connection]: data lose %u byte(s)", n_added);
        }

        ret = app_usbd_cdc_acm_read(&usb_cdc_acm, rx_buffer, NRFX_ARRAY_SIZE(rx_buffer));

        // while data is just copied from the internal buffer without initiating reading
      } while (ret == NRFX_SUCCESS);

      // In this case IO was not requested and no more RX will be done
      NRFX_ASSERT(ret != NRF_ERROR_BUSY);

      // Cannot ASSERT on NRF_ERROR_IO_PENDING, because NRF_ERROR_INVALID_STATE might occur

      break;
    }

    default:
      break;
  }
}

static void usb_connection_setup(void)
{
  const app_usbd_class_inst_t * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
  ret_code_t ret = app_usbd_class_append(class_cdc_acm);

  APP_ERROR_CHECK(ret);
}

static void wait_until_tx_done(void)
{
  while (m_is_tx_pending)
  {
    while (app_usbd_event_queue_process())
      ;
  }
}

void usb_connection_init(void)
{
  NRFX_ASSERT(LOG_BACKEND_USB_ENABLED);

  usb_connection_setup();
}

bool usb_connection_port_is_open(void)
{
  return m_is_port_open;
}

void usb_connection_process(void)
{
  app_usbd_event_queue_process();
}

size_t usb_connection_read(char * p_buffer, size_t read_size)
{
  NRFX_ASSERT(p_buffer != NULL);

  size_t copied = nrf_queue_out(&m_rx_queue, p_buffer, read_size);

  return copied;
}

int usb_connection_read_char(void)
{
  char ch;

  if (nrf_queue_pop(&m_rx_queue, &ch) == NRF_SUCCESS)
  {
    return ch;
  }
  else
  {
    return INVALID_CHAR;
  }
}

void usb_connection_write(const char * p_buffer, size_t length)
{
  NRFX_ASSERT(p_buffer != NULL);
  NRFX_ASSERT(m_is_port_open);

  size_t written_chars = 0;
  size_t left_chars    = length;

  NRF_LOG_DEBUG("[usb_connection]: new write buffer %u byte(s)", length);

  while (left_chars > 0)
  {
    size_t pushed_chars = nrf_queue_in(&m_tx_queue, &p_buffer[written_chars], left_chars);

    written_chars += pushed_chars;
    left_chars    -= pushed_chars;

    if (nrf_queue_is_full(&m_tx_queue))
    {
      usb_connection_flush();
      wait_until_tx_done();
    }
  }
}

void usb_connection_write_char(char ch)
{
  NRFX_ASSERT(m_is_port_open);
  ret_code_t ret = nrf_queue_push(&m_tx_queue, &ch);
  APP_ERROR_CHECK(ret);

  if (nrf_queue_is_full(&m_tx_queue))
  {
    usb_connection_flush();
    wait_until_tx_done();
  }
}

void usb_connection_flush(void)
{
  static char tx_buffer[NRFX_USBD_EPSIZE];

  while (!nrf_queue_is_empty(&m_tx_queue))
  {
    size_t bytes_to_send = nrf_queue_out(&m_tx_queue, tx_buffer, NRFX_USBD_EPSIZE);

    m_is_tx_pending = true;
    ret_code_t ret = app_usbd_cdc_acm_write(&usb_cdc_acm, tx_buffer, bytes_to_send);

    NRF_LOG_HEXDUMP_DEBUG(tx_buffer, bytes_to_send);

    if (ret == NRF_SUCCESS)
    {
      wait_until_tx_done();
    }
    else
    {
      NRF_LOG_ERROR("[usb_connection]: Flushing failure. %d byte(s) lost.", bytes_to_send);

      m_is_tx_pending = false; // return flag status
    }
  }
}
