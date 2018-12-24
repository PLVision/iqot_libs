#pragma once

/*=============================================================================
*
* The file is part of IQoT project which release under Apache 2.0 license.
* See the License file in the root of project.
* _____________________________________________________________________________
* Source code of this file uses pieces of code from ESP-IDF project
* https://github.com/espressif/esp-idf/
*
=============================================================================*/

#ifndef GATT_H

#define GATT_H


#include "esp_gatts_api.h"

extern uint32_t _transport_id;
extern struct gatts_profile_inst test_profile;

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/*
    This two who functions added by Plvision team for IQoT project.
    Set IQoT uuids and parameters for gatt and callback for iqot library.
*/
void set_gatt_parameters();
bool send_message(const void* data, size_t len);

#endif 