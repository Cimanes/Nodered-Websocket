### Components
1) Node-red server running in PC.
2) ESP8266 connected to BME280.

Connection between the ESP and Nodered via websockets.

## ESP8266
Coded in Arduino enviroment using VS code.
Following libraries are used: 
- Simpletimer 
- ArduinoJson
- ESP Async Webserver
- Adafruit BME280 Library
- Adafruit Unified Sensor
- Websockets

### Wifi Connection
- Option 1: Wifi Manager ("#define wifiManager" in wifi.hpp)
  Wifi credentials are stored as text files in LittleFS.

  If files do not exist, the ESP will reboot as server and prompt the user for credentials.
  The user needs to connect to "ESP WIFI MANAGER". 
  Then open the url 192.168.4.1 in a browser and enter the correct credentials.
  Once the credentials are entered, the ESP will reboot and connect to the Wifi as client.

* Option 2: Fixed Wifi credentials
  

## Node-red
Two flows are present:
- BME readings: displays current and historic BME signals-
- Console: user can monitor and operate GPIO's, and ethernet configuration. 

### BME Readings
The ESP periodically reads pressure, temperature and humidity and sends those values to Node-red.
When Node red receives the signals, and each of them feeds a chart and a dial gauge.

### Sea Level Pressure:
Local pressure reading is converted in Node-red to "Sea Level Pressure" (SLP). 

