#include "ble.h"
#include "ble_advertising.h"
#include "nrf_log.h"
#include "app_error.h"

#include "lib/utils.h"
#include "ble/stack/config.h"

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
  switch (ble_adv_evt)
  {
    case BLE_ADV_EVT_FAST:
      NRF_LOG_INFO("ADV Event: Start fast advertising");
      break;

    case BLE_ADV_EVT_IDLE:
      NRF_LOG_INFO("ADV Event: idle, no connectable advertising is ongoing");
      break;

    default:
      break;
  }
}

void ble_app_adverts_init(ble_advertising_t * advertising,
                          ble_uuid_t        * adv_uuid,
                          size_t              adv_uuid_len)
{
  ble_advertising_init_t init =
  {
    .advdata.name_type = BLE_ADVDATA_FULL_NAME,
    .advdata.flags     = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,

    .srdata.uuids_complete.p_uuids  = adv_uuid,
    .srdata.uuids_complete.uuid_cnt = adv_uuid_len,

    .config.ble_adv_fast_enabled  = true,
    .config.ble_adv_fast_interval = BLE_APP_ADV_INTERVAL,
    .config.ble_adv_fast_timeout  = BLE_APP_ADV_DURATION,

    .evt_handler = on_adv_evt,
  };

  CHECKED(ble_advertising_init(advertising, &init));

  ble_advertising_conn_cfg_tag_set(advertising, BLE_APP_CONN_CFG_TAG);
}

