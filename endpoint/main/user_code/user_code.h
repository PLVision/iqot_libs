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


#include "iqot_lib_api.h"

// pins for LED and dht sensor
#define LED 5
#define DHT_SIG 4

// iqot transport ID. This value needed for transport registration. Your endpoint can have
// a lot of communication transports, like bluetooth, lora, wifi, etc.

uint32_t _transport_id;

// this is struct for sensors. Iqot library register sensors, and you need to create variable for storing
// sensor ID. This is just example. You can store your ID for library as you like.
struct {
    uint32_t id;
    int16_t data;
} temperature_sensor, humidity_sensor;

// endpoint action
void led_on(const iqot_data_t args[]);
void led_off(const iqot_data_t args[]);
void update_parameters(void *pvParameter);