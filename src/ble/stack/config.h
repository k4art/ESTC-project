#ifndef BLE_APP_CONFIG_H__
#define BLE_APP_CONFIG_H__

#include "app_util.h"

#define BLE_APP_DEVICE_NAME             "ESTC Color Picker"

#define BLE_APP_ADV_INTERVAL            MSEC_TO_UNITS(200,  UNIT_0_625_MS)
#define BLE_APP_ADV_DURATION            MSEC_TO_UNITS(18000, UNIT_10_MS)

#define BLE_APP_OBSERVER_PRIO           3
#define BLE_APP_CONN_CFG_TAG            1

#define BLE_APP_MIN_CONN_INTERVAL       MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define BLE_APP_MAX_CONN_INTERVAL       MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define BLE_APP_SLAVE_LATENCY           0
#define BLE_APP_CONN_SUP_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)

#define BLE_APP_APPEARANCE              0x07C2 /* LED Lamp */

#define BLE_APP_ADV_UUIDS_COUNT         2
#define BLE_APP_ADV_DEV_INFO_UUID_IDX   0
#define BLE_APP_ADV_LED_SERV_UUID_IDX   1

#endif
