## Introduction

### Components
1) Node-red server running in PC / Raspberry pi.
2) ESP8266 with BME280 and digital outputs available.

### Communication
Connection between the ESP and Nodered via websockets, in url "/ws" port 1880.
The PC / Raspberry pi acts as server; The ESP is a client.
Ensure port 1880 is open; if required, add a new inbound rule in Windows Defender Firewall - Advanced settings.

## ESP8266
Coded in Arduino enviroment using VS code.
Following libraries are used: 
- Simpletimer 
- ArduinoJson
- ESP Async Webserver (requires ESPAsyncTCP).
- Adafruit BME280 Library
- Adafruit Unified Sensor
- Websockets
- AsyncElegantOTA

### Wifi Connection
- Option 1: Wifi Manager (enable line "#define WIFI_MANAGER")
  Wifi credentials are stored as text files in LittleFS.

  If files do not exist, the ESP will reboot as server and prompt the user for credentials.
  The user needs to connect to "ESP WIFI MANAGER". 
  Then open the url 192.168.4.1 in a browser and enter the correct credentials.
  Once the credentials are entered, the ESP will reboot and connect to the Wifi as client.

* Option 2: Fixed Wifi credentials (comment line "#define WIFI_MANAGER")
Wifi credentials need to be hard coded in wifi.hpp.
  
## Node-red
Three sub-flows are present:
- BME readings: displays current and historic BME signals (P, S.L.P., T, RH, time).
- Console: user can monitor and operate GPIO's, thermostat and ESP (Reboot, OTA update...).
- Console feedback: receive actual status of GPIO's and BME. 

### BME Readings
The ESP periodically reads pressure, temperature and humidity and sends those values to Node-red.
When Node red receives the signals, and each of them feeds a chart and a dial gauge.
Pressure is converted to Sea Level Pressure (SLP)- Local elevation needed.
