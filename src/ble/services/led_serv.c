#include "ble/stack/config.h"
#include "ble_srv_common.h"

#include "lib/utils.h"
#include "domain/events/events.h"
#include "domain/colors/colors.h"
#include "domain/color_picker/color_picker.h"

#include "led_serv.h"
#include "nrf_sdh_ble.h"

static ble_app_serv_led_t m_ble_app_serv_led =
{
  .conn_handle = BLE_CONN_HANDLE_INVALID,
};

static ret_code_t add_led_color_char(ble_app_serv_led_t * serv)
{
  ble_uuid_t char_uuid =
  {
    .uuid = BLE_APP_SERV_LED_COLOR_CHAR_UUID,
    .type = serv->uuid_type,
  };

  ble_srv_utf8_str_t user_desc;

  ble_srv_ascii_to_utf8(&user_desc, BLE_APP_SERV_LED_COLOR_CHAR_USER_DESC);

  ble_gatts_attr_md_t attr_md = { .vloc = BLE_GATTS_VLOC_STACK };
  ble_gatts_attr_md_t cccd_md = { .vloc = BLE_GATTS_VLOC_STACK };

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

  ble_gatts_char_md_t char_md =
  {
    .char_props              = { .notify = 1, .read = 1, .write = 1 },
    .p_char_user_desc        = user_desc.p_str,
    .char_user_desc_size     = user_desc.length,
    .char_user_desc_max_size = user_desc.length,
    .p_cccd_md               = &cccd_md,
  };

  ble_gatts_attr_t attr_char_value =
  {
    .p_uuid     = &char_uuid,
    .p_attr_md  = &attr_md,
    .max_len    = sizeof(rgb_color_t),
    .init_len   = sizeof(rgb_color_t),
  };

  return sd_ble_gatts_characteristic_add(serv->service_handle,
                                         &char_md,
                                         &attr_char_value,
                                         &serv->char_color_handles);
}

static void set_char_led_color_value(rgb_color_t color)
{
  ble_gatts_value_t value =
  {
    .len     = sizeof(color),
    .offset  = 0,
    .p_value = (uint8_t *) &color,
  };

  sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID,
                         m_ble_app_serv_led.char_color_handles.value_handle,
                         &value);
}

static void sync_char_led_color_value(app_events_evt_t evt, uint32_t data)
{
  ble_app_serv_led_t * serv = &m_ble_app_serv_led;
  rgb_color_t color = word_to_rgb(data);

  if (serv->conn_handle != BLE_CONN_HANDLE_INVALID)
  {
    ble_gatts_hvx_params_t hvx_params =
    {
      .handle = serv->char_color_handles.value_handle,
      .type   = BLE_GATT_HVX_NOTIFICATION,
      .offset = 0,
      .p_len  = &(uint16_t) { sizeof(rgb_color_t) },
      .p_data = (uint8_t *) &color,
    };
    
    sd_ble_gatts_hvx(serv->conn_handle, &hvx_params);
  }
  else
  {
    // If there is no connection, the attribute value should be update for consistency,
    // so that a new connection will read up-to-date color value.

    // TODO: Maintaining the attribute value in case of absence of a connection
    // is not necessary. An alternative to this might be accomplished as follows:
    // 1. Color Picker allows to read its current color
    // 2. On new connection event get the color and call set_char_led_color_value()

    set_char_led_color_value(color);
  }
}

ret_code_t ble_app_serv_led_init(ble_uuid_t * p_uuid)
{
  NRFX_ASSERT(p_uuid != NULL);

  ble_uuid128_t base_uuid = { .uuid128 = BLE_APP_SERV_LED_BASE_UUID };

  CHECKED(sd_ble_uuid_vs_add(&base_uuid, &m_ble_app_serv_led.uuid_type));

  p_uuid->uuid = BLE_APP_SERV_LED_UUID;
  p_uuid->type = m_ble_app_serv_led.uuid_type;

  CHECKED(sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                   p_uuid,
                                   &m_ble_app_serv_led.service_handle));

  CHECKED(add_led_color_char(&m_ble_app_serv_led));

  NRF_SDH_BLE_OBSERVER(ble_app_serv_led,
                       BLE_APP_OBSERVER_PRIO,
                       ble_app_serv_led_evt_handler,
                       NULL);

  return app_events_on(APP_EVENTS_EVT_LED_SYNC, sync_char_led_color_value);
}

static void handle_led_color_write(const ble_gatts_evt_write_t * evt)
{
  if (evt->len != 3 || evt->offset != 0)
  {
    NRF_LOG_INFO("The %s characterstic should be written with len = 3 and offset = 0",
                 BLE_APP_SERV_LED_COLOR_CHAR_USER_DESC);

    // FIXME: the color is not changed (because of this if-else), but
    // the attribute value is modified. It should be synchronized  with Color Picker,
    // because the next connection might read incorrect color value.

    return;
  }
  else
  {
    rgb_color_t rgb = * (rgb_color_t *) &evt->data;

    color_picker_set_hsv(rgb_to_hsv(rgb));
  }
}

void ble_app_serv_led_evt_handler(const ble_evt_t * ble_evt, void * context)
{
  (void) context;
  
  ble_app_serv_led_t * serv = &m_ble_app_serv_led;
  
  switch (ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
      serv->conn_handle = ble_evt->evt.gap_evt.conn_handle;
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      serv->conn_handle = BLE_CONN_HANDLE_INVALID;
      break;

    case BLE_GATTS_EVT_WRITE:
    {
      const ble_gatts_evt_write_t * evt = &ble_evt->evt.gatts_evt.params.write;

      if (evt->uuid.uuid == BLE_APP_SERV_LED_COLOR_CHAR_UUID)
      {
        handle_led_color_write(evt);
      }
      break;
    }
  }
}

