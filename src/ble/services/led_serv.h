#ifndef ESTC_BLE_APP_LED_COLOR_SERV_H__
#define ESTC_BLE_APP_LED_COLOR_SERV_H__

#include <stdint.h>

#include "ble.h"
#include "ble_gatts.h"
#include "app_error.h"

#include "lib/utils.h"

#define BLE_APP_SERV_LED_BASE_UUID   UUID_BIGEND(9c570000, f5be, 40f1, b183, 5bec26002180)

#define BLE_APP_SERV_LED_UUID                     0x0001
#define BLE_APP_SERV_LED_COLOR_READER_CHAR_UUID   0x1000
#define BLE_APP_SERV_LED_COLOR_WRITER_CHAR_UUID   0x1001

#define BLE_APP_SERV_LED_COLOR_READER_CHAR_USER_DESC   "LED color"
#define BLE_APP_SERV_LED_COLOR_WRITER_CHAR_USER_DESC   "Change LED color"

typedef struct ble_app_serv_led_s
{
  uint16_t                    service_handle;
  uint16_t                    conn_handle;
  uint8_t                     uuid_type;
  ble_gatts_char_handles_t    reader_char_handles;
  ble_gatts_char_handles_t    writer_char_handles;
} ble_app_serv_led_t;

ret_code_t ble_app_serv_led_init(ble_uuid_t * p_uuid);

void ble_app_serv_led_evt_handler(const ble_evt_t * p_ble_evt, void * context);

#endif

