#ifndef BLE_APP_ADVERTS_H__
#define BLE_APP_ADVERTS_H__

#include "ble_advertising.h"

void ble_app_adverts_init(ble_advertising_t * advertising,
                          ble_uuid_t        * adv_uuid,
                          size_t              adv_uuid_len);

#endif

