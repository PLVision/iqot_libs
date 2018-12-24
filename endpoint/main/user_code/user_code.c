/*=============================================================================
*
* The file is part of IQoT project which release under Apache 2.0 license.
* See the License file in the root of project.
* _____________________________________________________________________________
* Copyright (c) 2018 PLVision sp. z o.o. All Rights Reserved.
*
* NOTICE: All information contained herein is, and remains the property of
* PLVision sp. z o.o. and its suppliers, if any. The intellectual and technical
* concepts contained herein are proprietary to PLVision sp. z o.o. and its
* suppliers, and are protected by trade secret or copyright law. Dissemination
* of this information or reproduction of this material is strictly forbidden
* unless prior written permission is obtained from PLVision.
*
=============================================================================*/

#include "user_code.h"
#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void led_on(const iqot_data_t args[])
{
    gpio_set_level(LED, 1);
}

void led_off(const iqot_data_t args[])
{
    gpio_set_level(LED, 0);
}

void update_parameters(void *pvParameter)
{
    while (1)
    {
        dht_read_data(DHT_TYPE_DHT11, DHT_SIG, &humidity_sensor.data, &temperature_sensor.data);

        // -- wait at least 2 sec before reading again 
        vTaskDelay(2000 / portTICK_RATE_MS);

        // send telemetry data to registered notification client via GATT notification
        // "iqot_sensor_data_update" will call callback function for sending data by BLE
        iqot_sensor_data_update(humidity_sensor.id,(iqot_data_t){.int32_value = humidity_sensor.data});
        vTaskDelay(2);
        iqot_sensor_data_update(temperature_sensor.id, (iqot_data_t){.int32_value = temperature_sensor.data});
        vTaskDelay(3000 / portTICK_RATE_MS);
    }
}