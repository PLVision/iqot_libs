### Description

This repository contains software to bring up real or emulated devices and connect them to IQoT platform:
https://iqot.io/

##### Emulator
Contains scripts to download two docker images with gateway and endpoint emulators.
It emulates running gateway with one connected endpoint.

Validated on Ubuntu 16.04 64-bit.

##### Endpoint
Contains scripts to install ESP-IDF environment and flash ESP32 bord with BLE sample.
BLE sample contains IQoT library, modules for BLE and DHT sensor as a sample of IQoT library usage.
Supported devices: ESP32 Lora SX1278 or ESP32 Lora SX1276.

##### Gateway
Contains script to download two docker images: core application and network driver.

Main responsibilities:
* Establishing connection with the IQoT cloud application
* Receiving actions from the IQoT cloud application targeted at endpoints
* Sending telemetry data to the IQoT cloud application received from a network driver

Validated on - Raspberry Pi 3 Model B.
