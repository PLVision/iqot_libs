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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "driver/gpio.h"
#include "esp_bt_device.h"

#include "iqot_lib_api.h"
#include "gatt.h"
#include "gap.h"
#include "user_code/user_code.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"


#define BLE_INFO "BLE_INFO"
/* application ID */

#define PROFILE_ON_APP_ID 0

void app_main() 
{
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 0);

    /* initialize profile and characteristic permission and property*/
    set_gatt_parameters();

    esp_err_t ret;

    ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(ret);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    if (esp_bt_controller_init(&bt_cfg))
    {
        ESP_LOGE(BLE_INFO, "%s initialize controller failed\n", __func__);
        return;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM))
    {
        ESP_LOGE(BLE_INFO, "%s enable controller failed\n", __func__);
        return;
    }

    if (esp_bluedroid_init())
    {
        ESP_LOGI(BLE_INFO, "\n%s initialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bluedroid_enable()) 
    {
        ESP_LOGI(BLE_INFO, "\n%s enable bluedroid failed\n", __func__);
        return;
    }

    /* register callbacks to handle events of GAp and GATT*/
    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);
    /* register profiles with app id */
    esp_ble_gatts_app_register(PROFILE_ON_APP_ID);

    /* BLE security parameters configuration */

    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND; 
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;       /* set the IO capability to No output No input */
    uint8_t key_size = 16;                          /* the key size should be 7~16 bytes */
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;

    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));

    /* ===============Connection to IQoT is started from Here. =================== */

    /* mac address of bt device */
    const uint8_t* mac = esp_bt_dev_get_address();

    /* MAC address for whitelist */
    ESP_LOGW(BLE_INFO, "***************************************"); 
    ESP_LOGW(BLE_INFO, "BT mac for whitelist: %x:%x:%x:%x:%x:%x ", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGW(BLE_INFO, "*************************************** \n\n"); 
    /* Endpoint ID for IQoT web app */
    ESP_LOGE(BLE_INFO, "********************************************"); 
    ESP_LOGE(BLE_INFO, "Endpoint ID for IQoT web app: ep%x%x%x%x%x%x ", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGE(BLE_INFO, "********************************************\n\n");

    /* register transport and set callback for sending data */ 
    iqot_error_t err;
    iqot_config_t iqot_config; 
    err = iqot_init(&iqot_config); /* init parameters for transport */ 
    if (err == IQOT_ERROR_INTERNAL_ERROR)
    {
        ESP_LOGE(BLE_INFO, "error in iqot init");
    }
    
    iqot_transport_cb_t tr_config;
    tr_config.send_data = send_message; /* set callback for sending */
    err = iqot_register_transport(&tr_config, &_transport_id); /* register transport */ 

    if (err == IQOT_ERROR_INTERNAL_ERROR)
    {   
        ESP_LOGE(BLE_INFO, "error in reqister transport");
    }
    /*  
        set state of transport. In the beginnig BLE transport is disabled. After bonding with device 
        iqot lib will change connection state
    */
    iqot_connection_state_changed(_transport_id, IQOT_CONNECTION_STATE_DISCONNECTED);
    /* registering sensors */
    
    err = iqot_register_sensor("humidity", IQOT_DATA_TYPE_INT, &humidity_sensor.id);
    if (err == IQOT_ERROR_INVALID_PARAMETER)
    {   
        ESP_LOGE(BLE_INFO, "humidity sensor already exist");
    }

    err = iqot_register_sensor("temperature", IQOT_DATA_TYPE_INT, &temperature_sensor.id);
    if (err == IQOT_ERROR_INVALID_PARAMETER)
    {   
        ESP_LOGE(BLE_INFO, "temperature sensor already exist");
    }

    /* registering actions */
    err = iqot_register_action("led_on", led_on, 0);
    if (err == IQOT_ERROR_INVALID_PARAMETER)
    {   
        ESP_LOGE(BLE_INFO, "led_on action already exist");
    }

    err = iqot_register_action("led_off", led_off, 0);
    if (err == IQOT_ERROR_INVALID_PARAMETER)
    {   
        ESP_LOGE(BLE_INFO, "led_off action already exist");
    }

    /* run task for telemetry update */
    xTaskCreate(&update_parameters, "update_parameters", 2048, NULL, 5, NULL);  

    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}