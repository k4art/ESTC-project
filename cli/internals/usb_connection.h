#ifndef USB_OCNNECTION_H
#define USB_OCNNECTION_H

typedef void (* usb_connection_read_done_handler_t)(void);
typedef void (* usb_connection_write_done_handler_t)(void);

void usb_connection_init(void);

bool usb_connection_port_is_open(void);

void usb_connection_process(void);

/* The length of p_buffer must be at least read_size. */
size_t usb_connection_read(char * p_buffer, size_t read_size);

int usb_connection_read_char(void);

void usb_connection_write(const char * p_buffer, size_t buffer_size);

void usb_connection_write_char(char ch);

void usb_connection_rx_clear(void);

void usb_connection_flush(void);

#endif
