#include "ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"

#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"

#include "nrfx.h"
#include "nrf_log.h"
#include "app_error.h"
#include "app_timer.h"

#include "lib/utils.h"
#include "ble/stack/config.h"
#include "ble/stack/init.h"
#include "ble/stack/adverts.h"
#include "ble/services/led_serv.h"

NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);
BLE_ADVERTISING_DEF(m_advertising);

static ble_uuid_t m_adv_uuids[BLE_APP_ADV_UUIDS_COUNT] =
{
  [BLE_APP_ADV_DEV_INFO_UUID_IDX] =
  {
    .uuid = BLE_UUID_DEVICE_INFORMATION_SERVICE,
    .type = BLE_UUID_TYPE_BLE,
  },
};

static void ble_app_stack_init(void)
{
  uint32_t ram_start = 0;
  
  CHECKED(nrf_sdh_enable_request());
  CHECKED(nrf_sdh_ble_default_cfg_set(BLE_APP_CONN_CFG_TAG, &ram_start));
  CHECKED(nrf_sdh_ble_enable(&ram_start));
}

static void gap_params_init(void)
{
  ble_gap_conn_sec_mode_t sec_mode;
  ble_srv_utf8_str_t      device_name;

  ble_gap_conn_params_t   gap_conn_params =
  {
    .min_conn_interval = BLE_APP_MIN_CONN_INTERVAL,
    .max_conn_interval = BLE_APP_MAX_CONN_INTERVAL,
    .slave_latency     = BLE_APP_SLAVE_LATENCY,
    .conn_sup_timeout  = BLE_APP_CONN_SUP_TIMEOUT,
  };

  ble_srv_ascii_to_utf8(&device_name, BLE_APP_DEVICE_NAME);

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  CHECKED(sd_ble_gap_device_name_set(&sec_mode, device_name.p_str, device_name.length));
  CHECKED(sd_ble_gap_appearance_set(BLE_APP_APPEARANCE));
  CHECKED(sd_ble_gap_ppcp_set(&gap_conn_params));
}

static void services_init(void)
{
  nrf_ble_qwr_init_t qwr_init = {0};

  // Initialize Queued Write Module.
  // qwr_init.error_handler = nrf_qwr_error_handler;

  CHECKED(nrf_ble_qwr_init(&m_qwr, &qwr_init));
  CHECKED(ble_app_serv_led_init(&m_adv_uuids[BLE_APP_ADV_LED_SERV_UUID_IDX]));
}

static void pm_evt_handler(pm_evt_t const * p_evt)
{
  pm_handler_on_pm_evt(p_evt);
  pm_handler_disconnect_on_sec_failure(p_evt);
  pm_handler_flash_clean(p_evt);
}

static void peer_manager_init(void)
{
  // "Just Works Bonding"
  // Source: https://infocenter.nordicsemi.com/index.jsp?topic=%2Fsdk_nrf5_v17.1.0%2Flib_pm_usage.html
  ble_gap_sec_params_t sec_params =
  {
    .bond           = true,
    .io_caps        = BLE_GAP_IO_CAPS_NONE,
    .min_key_size   = 7,
    .max_key_size   = 16,
    .kdist_own.enc  = 1,
    .kdist_own.id   = 1,
    .kdist_peer.enc = 1,
    .kdist_peer.id  = 1,
  };

  CHECKED(pm_init());
  CHECKED(pm_sec_params_set(&sec_params));
  CHECKED(pm_register(pm_evt_handler));
}

static void initialize(void)
{
  ble_app_stack_init();
  gap_params_init();
  CHECKED(nrf_ble_gatt_init(&m_gatt, NULL));
  services_init();

  ble_app_adverts_init(&m_advertising,
                       m_adv_uuids,
                       NRFX_ARRAY_SIZE(m_adv_uuids) - 1);

  peer_manager_init();
}

void ble_app_startup(void)
{
  initialize();
  
  CHECKED(ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST));
}

