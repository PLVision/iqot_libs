#pragma once



#ifndef GAP_H

#define GAP_H

#include "esp_gap_ble_api.h"

extern uint32_t _transport_id;

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif 