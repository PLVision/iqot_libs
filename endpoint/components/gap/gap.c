/*=============================================================================
*
* The file is part of IQoT project which release under Apache 2.0 license.
* See the License file in the root of project.
* _____________________________________________________________________________
* Source code of this file uses pieces of code from ESP-IDF project
* https://github.com/espressif/esp-idf/
*
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

/*This include added by Plvision Team for IQoT project.*/
#include "iqot_lib_api.h"
#include "gap.h"

#define GAP_TAG "GAP_INFO"

static uint8_t service_uuid128 [ESP_UUID_LEN_128] = { 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static esp_ble_adv_data_t test_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = ESP_UUID_LEN_128,
    .p_service_uuid = service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static char *esp_key_type_to_str(esp_ble_key_type_t key_type)
{
    char *key_str = NULL;
    switch (key_type)
    {
    case ESP_LE_KEY_NONE:
        key_str = "ESP_LE_KEY_NONE";
        break;
    case ESP_LE_KEY_PENC:
        key_str = "ESP_LE_KEY_PENC";
        break;
    case ESP_LE_KEY_PID:
        key_str = "ESP_LE_KEY_PID";
        break;
    case ESP_LE_KEY_PCSRK:
        key_str = "ESP_LE_KEY_PCSRK";
        break;
    case ESP_LE_KEY_PLK:
        key_str = "ESP_LE_KEY_PLK";
        break;
    case ESP_LE_KEY_LLK:
        key_str = "ESP_LE_KEY_LLK";
        break;
    case ESP_LE_KEY_LENC:
        key_str = "ESP_LE_KEY_LENC";
        break;
    case ESP_LE_KEY_LID:
        key_str = "ESP_LE_KEY_LID";
        break;
    case ESP_LE_KEY_LCSRK:
        key_str = "ESP_LE_KEY_LCSRK";
        break;
    default:
        key_str = "INVALID BLE KEY TYPE";
        break;
    }
    return key_str;
}

static void show_bonded_devices(void)
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    ESP_LOGI(GAP_TAG, "Bonded devices number : %d\n", dev_num);

    ESP_LOGI(GAP_TAG, "Bonded devices list : %d\n", dev_num);
    for (int i = 0; i < dev_num; i++)
    {
        esp_log_buffer_hex(GAP_TAG, (void *)dev_list[i].bd_addr, sizeof(esp_bd_addr_t));
    }

    free(dev_list);
}

static void __attribute__((unused)) remove_all_bonded_devices(void)
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    for (int i = 0; i < dev_num; i++)
    {
        esp_ble_remove_bond_device(dev_list[i].bd_addr);  
    }

    free(dev_list);
}

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    ESP_LOGV(GAP_TAG, "GAP_EVT, event %d\n", event);
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT\n");
        break;

    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT\n");
        break;

    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:

         ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT\n");
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:

         ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT %d\n",param->adv_start_cmpl.status);
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GAP_TAG, "\nAdvertising start failed\n");
        }else
        {
            ESP_LOGI(GAP_TAG, "\nAdvertising start\n");

            /*
                This call added by Plvision Team for IQoT project.
                If advertising start successful, transport state will be changed to disconnected
                and all bonded devices will be removed.
            */
            iqot_connection_state_changed(_transport_id, IQOT_CONNECTION_STATE_DISCONNECTED);
            remove_all_bonded_devices();
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:

        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(GAP_TAG, "\nAdvertising stop failed\n");
        }
        else
        {
            ESP_LOGI(GAP_TAG, "\nStop adv successfully\n");
        }
        break;

    case ESP_GAP_BLE_PASSKEY_REQ_EVT: /* passkey request event */
        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        break;
    case ESP_GAP_BLE_OOB_REQ_EVT: /* OOB request event */

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        break;

    case ESP_GAP_BLE_LOCAL_IR_EVT: /* BLE local IR event */

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;

    case ESP_GAP_BLE_LOCAL_ER_EVT: /* BLE local ER event */

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;

    case ESP_GAP_BLE_NC_REQ_EVT:
        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
        break;
        
    case ESP_GAP_BLE_SEC_REQ_EVT:

         ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SEC_REQ_EVT\n");

        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: 

        ESP_LOGI(GAP_TAG, "The passkey Notify number:%d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_KEY_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_KEY_EVT");
        //shows the ble key info share with peer device to the user.
        ESP_LOGI(GAP_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        
        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_AUTH_CMPL_EVT");
        esp_bd_addr_t bd_addr;
        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GAP_TAG, "remote BD_ADDR: %08x%04x",
                 (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                 (bd_addr[4] << 8) + bd_addr[5]);
        ESP_LOGI(GAP_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(GAP_TAG, "pair status = %s", param->ble_security.auth_cmpl.success ? "success" : "fail");
        show_bonded_devices();

        /*
            This call added by Plvision Team for IQoT project.
            If advertising start successful, transport state will be changed to connected
        */
        iqot_connection_state_changed(_transport_id, IQOT_CONNECTION_STATE_CONNECTED);
        break;
    
    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:

        ESP_LOGD(GAP_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d", param->remove_bond_dev_cmpl.status);
        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
        ESP_LOGI(GAP_TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
        esp_log_buffer_hex(GAP_TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GAP_TAG, "------------------------------------");
        break;

    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "\nESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT\n");
        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GAP_TAG, "config local privacy failed, error status = %x", param->local_privacy_cmpl.status);
            break;
        }

        esp_err_t ret = esp_ble_gap_config_adv_data(&test_adv_data);

        if (ret)
        {
            ESP_LOGE(GAP_TAG, "config adv data failed, error code = %x", ret);
        }

        break;

    case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SCAN_RESULT_EVT");
        break;

    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT");
        break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT status = %x",param->update_conn_params.status);
        esp_log_buffer_hex(GAP_TAG, (void *)param->update_conn_params.bda, sizeof(esp_bd_addr_t));
        break;

    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT");
        break;

    case ESP_GAP_BLE_EVT_MAX:

        ESP_LOGI(GAP_TAG, "ESP_GAP_BLE_EVT_MAX");
        break;

    default:
         ESP_LOGE(GAP_TAG, "unknown GAP event %x\n", event);
        break;
    }
}