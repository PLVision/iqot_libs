# BLE endpoint Demo  



## Install environment  

*Note:you can skip this step*	
if you already have the esp-idf environment 
1. Run the `sudo bash install_env.sh $USER` command 

2. Log out of your ubuntu user account and log in again.

## Setup device: 

for this purpose you need:  
1. ESP32 Lora SX1278 or ESP32 Lora SX1276  
*(you can also use any esp32 board with BLE chip but please note that pins in code of BLE sample are to be rewritten)*  
2. DHT11 module
3. Resistor 300 ohm
4. LED

Connection scheme:  


| ESP32 pin num | DHT11 | resistor     |    LED    |
| ------------- | ----- | -------------|-----------| 
| VCC +3.3V 	| +3.3V |              |           |
| GND           | GND   |              |     -     |
| 4             | SIGNAL|              |           |
| 5             |       |   1th leg    |           |
|               |       |   2th leg    |     +     |

*you can find a visual scheme in the endpoint folder*

## Build and flash firmware  
			
1. Plug your endpoint to USB port

2. Assign permissions for ttyUSB0 port using the following command:
`sudo chmod 777 /dev/ttyUSB0`

3. Type the `make flash monitor’ in the endpoint directory. After flashing you will see logs from serial.

4. In the console you can see mac of your BLE chip and ID to be used in the IQoT web platform. Remember this mac and ID.

5. Finally, you can unplug your device. 

## Setup endpoint on IQoT  

1. First you need to know the ID of your Endpoint. if you flash and monitor your device using the "make flash monitor" command, you can see your ID in logs.

2. Secondly, you need MAC of your BLE module for gateway whitelist. Without entering MAC to the gateway whitelist, your gateway will do not find your endpoint.

3. Before plugging your device on, create your device in [iqot.io](https://iqot.io/devices/). 
- Set name of device
- Set device ID 
- Set Type (endpoint)


# NOTE
		
1. It is very important to connect your device with a gateway after the device has been created in IQoT. If you connect an endpoint first, the important configuration data of an endpoint will  be lost.  

2. If you use a different board and after having flashed and monitored it by serial you see the unexpected output, change XTAL frequency to the board recommended frequency.   
`make menuconfig => component config => Esp32-specific => Main XTAL frequency`


