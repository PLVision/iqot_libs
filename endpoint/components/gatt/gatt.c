/*=============================================================================
*
* The file is part of IQoT project which release under Apache 2.0 license.
* See the License file in the root of project.
* _____________________________________________________________________________
*
* Source code of this file uses pieces of code from ESP-IDF project
* https://github.com/espressif/esp-idf/
*
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_gap_ble_api.h"

/* This include added by Plvision Team for IQoT project. */
#include "iqot_lib_api.h"
#include "gatt.h"

/* This define added by Plvision team for IQoT project. uuid of service. */
#define GATTS_SERVICE_UUID_TEST 0xAABB

/*
    Uuid of characteristic. This characteristic is tracked by gateway network driver. If you
    change uuid of this characteristic, you need to change it in gateway network driver too.
*/
#define GATTS_CHAR_UUID_TRANSPORT 0x2A6F
#define GATTS_NUM_HANDLES 12
#define GATTS_CHAR_VAL_LEN_MAX 22

#define TEST_DEVICE_NAME "ESP_DEMO"
/* maximum value of a characteristic */
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0xFF

#define CHAR_NUM 2
#define CHARACTERISTIC_TRANSPORT_ID 0

#define GATTS_TAG "GATT_INFO"

/* value range of a attribute (characteristic) */
uint8_t transport_str[GATTS_CHAR_VAL_LEN_MAX] = {0x11,0x22,0x33};

/* struct for characteristic value */
esp_attr_value_t gatts_transport_attr_val = {
        .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
        .attr_len = sizeof(transport_str),
        .attr_value = transport_str,
};

/* this variable holds advertising parameters */
static esp_ble_adv_params_t test_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_characteristic_inst
{
    esp_bt_uuid_t char_uuid; 
    uint16_t char_handle;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    esp_bt_uuid_t descr_uuid;
    uint16_t descr_handle;
};

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    struct gatts_characteristic_inst chars[CHAR_NUM];
};

/* this variable holds the information of current BLE connection */
struct gatts_profile_inst test_profile;

void process_write_event_env(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    
    if (param->write.need_rsp)
    {
        ESP_LOGI(GATTS_TAG, "respond");
        /*
            This call added by Plvision team for IQoT project.
            "iqot_data_received" take data from gateway and decode this. After successful decoding endpoint will execute action.
        */
        iqot_data_received(_transport_id, (char*)param->write.value, strlen((char*)param->write.value));
        esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        if (response_err != ESP_OK)
        {
            ESP_LOGI(GATTS_TAG, "\nSend response error\n");
        }
    }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGV(GATTS_TAG, "event = %x\n", event);
    switch (event)
    {
    /* When register application id, the event comes */
    case ESP_GATTS_REG_EVT:
    {
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        test_profile.service_id.is_primary = true;
        test_profile.service_id.id.inst_id = 0x00;
        test_profile.service_id.id.uuid.len = ESP_UUID_LEN_16;
        test_profile.service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST;
        esp_err_t ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
        if(ret)
        {
            ESP_LOGE(GATTS_TAG, "device name seting failed, error code = %x\n", ret);
        }else
        {
            ESP_LOGI(GATTS_TAG, "device name set successfully, code = %x\n", ret);
        }
        ret = esp_ble_gap_config_local_privacy(false);
        if(ret)
        {
            ESP_LOGE(GATTS_TAG, "local privacy config failed, error code = %x\n", ret);
        }
        /* after finishing registering, the ESP_GATTS_REG_EVT event comes, we start the next step is creating service */
        ret = esp_ble_gatts_create_service(gatts_if, &test_profile.service_id, GATTS_NUM_HANDLES);
        if (ret)
        {
            ESP_LOGE(GATTS_TAG, "service creating failed, error code = %x\n", ret);
        }
        break;
    }
    case ESP_GATTS_READ_EVT:
    {
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_READ_EVT\n");
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 2;
        rsp.attr_value.value[0] = 0;
        /* When central device send READ request to GATT server, the ESP_GATTS_READ_EVT comes */
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);

        break;
    }
    case ESP_GATTS_RESPONSE_EVT:
    {
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_RESPONSE_EVT\n");
        break;
    }
    /* ESP_GATTS_WRITE_EVT event will call "process_write_event_env" */
    case ESP_GATTS_WRITE_EVT:
    {
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_WRITE_EVT\n");
        process_write_event_env(gatts_if, param);
        break;
    }
    /* When create service complete, the event comes */
    case ESP_GATTS_CREATE_EVT:
    {
        ESP_LOGI(GATTS_TAG, "status %d, service_handle %x, service id %x\n", param->create.status, param->create.service_handle, param->create.service_id.id.uuid.uuid.uuid16);
        /* store service handle and add characteristics */
        test_profile.service_handle = param->create.service_handle;
        /* transport characteristic */
        esp_err_t ret = esp_ble_gatts_add_char(test_profile.service_handle,
                               &test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].char_uuid,
                               test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].perm,
                               test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].property,
                               &gatts_transport_attr_val, NULL);

        if (ret)
        {
            ESP_LOGE(GATTS_TAG, "failed to add characteristic CHARACTERISTIC_TRANSPORT_ID, error code = %x\n", ret);
        }
        /* and start service */
        ret = esp_ble_gatts_start_service(test_profile.service_handle);
        if (ret)
        {
            ESP_LOGE(GATTS_TAG, "failed to start service, error code = %x\n", ret);
        }
        break;
    }
    /* When add characteristic complete, the event comes */
    case ESP_GATTS_ADD_CHAR_EVT:
    {
        ESP_LOGI(GATTS_TAG, "\nADD_CHAR_EVT, status %d, attr_handle %x, service_handle %x, char uuid %x\n",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle, param->add_char.char_uuid.uuid.uuid16);
        /* store characteristic handles for later usage */
        if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_TRANSPORT)
        {
            test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].char_handle = param->add_char.attr_handle;
        }
        break;
    }    
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
    {
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_ADD_CHAR_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    }
    /* when disconneting, send advertising information again */
    case ESP_GATTS_DISCONNECT_EVT:
    {
        ESP_LOGI(GATTS_TAG, "\nESP_GATTS_DISCONNECT_EVT\n");
        /*
            This call added by Plvision team for IQoT project.
            Before advertising, transport state will be changed to disconnected.
        */
        iqot_connection_state_changed(_transport_id, IQOT_CONNECTION_STATE_DISCONNECTED);
        esp_ble_gap_start_advertising(&test_adv_params);
        break;
    }
    /* When gatt client connect, the event comes */
    case ESP_GATTS_CONNECT_EVT:
    {
        ESP_LOGI(GATTS_TAG, "\nESP_GATTS_CONNECT_EVT\n");
        esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
        break;
    }
    case ESP_GATTS_START_EVT:
    {
        ESP_LOGI(GATTS_TAG, "\nESP_GATTS_START_EVT\n");
        esp_err_t ret = esp_ble_gap_start_advertising(&test_adv_params);
        if(ret)
        {
             ESP_LOGE(GATTS_TAG, "failed to start advertising, error code = %x\n", ret);
        }
        break;
    }
    case ESP_GATTS_MTU_EVT:
    {
        ESP_LOGI(GATTS_TAG, "\nESP_GATTS_MTU_EVT\n");
        ESP_LOGI(GATTS_TAG ,"Connection id = %d  MTU size = %d" ,param->mtu.conn_id, param->mtu.mtu);
        break;
    }
    case ESP_GATTS_CONF_EVT:
            ESP_LOGI(GATTS_TAG, "\nESP_GATTS_CONF_EVT\n");
        break;
    default:
        ESP_LOGE(GATTS_TAG,"unknown GATT event\n");
        break;
    }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{   
    /* If event is register event, store the gatts_if for the profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            test_profile.gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "\nReg app failed, app_id %04x, status %d\n",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    if (gatts_if == ESP_GATT_IF_NONE ||
        gatts_if == test_profile.gatts_if)
    {
        if (test_profile.gatts_cb)
        {
            test_profile.gatts_cb(event, gatts_if, param);
        }
    }
}

/* 
    This function added by Plvision team for IQoT project.
    Callback for iqot library.
*/
bool send_message(const void* data, size_t len)
{
    esp_ble_gatts_send_indicate(test_profile.gatts_if,
                                test_profile.conn_id,
                                test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].char_handle,
                                len, (uint8_t*)data, false);

    return true;
}

/* This function added by Plvision team for IQoT project */

void set_gatt_parameters()
{
/*
    GATTS_CHAR_UUID_TRANSPORT is transport characteristic for iqot library. 
    Property of this characteristic have bit for notification and write.
    If you want create your own iqot characteristic, set notification and write bit to property

    P.S dont forget set "write" permisions for your own iqot characteristics.
*/
	test_profile.gatts_cb = gatts_profile_event_handler;
    test_profile.gatts_if = ESP_GATT_IF_NONE;
    test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].char_uuid.len = ESP_UUID_LEN_16;
    test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TRANSPORT;
    test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].perm = ESP_GATT_PERM_WRITE; /* write permisions for GATTS_CHAR_UUID_TRANSPORT characteristic */
    test_profile.chars[CHARACTERISTIC_TRANSPORT_ID].property = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_WRITE; /* set notification and write bit to characteristic property */
}
