//======================================
// LIBRARIES
//======================================
#include "00_globals.hpp"
#include "01_json.hpp"
#include "02_fileSys.hpp"
#include "03_wifi.hpp"
#include "04_bme.hpp"
#include "05_websocket.hpp"
#include "06_events.hpp"

//======================================
// SETUP
//======================================
void setup() {
  Serial.begin(115200);
  initFS()            ; // Initialize File System
  initGPIO()          ; // Initialize GPIO's
  initBME()           ; // Initialize BME sensor

  // Initialize wifi and Websocket; Send BME data after WS connected:
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onwifiDisconnect);
  wifiConnectHandler = WiFi.onStationModeGotIP(onwifiConnect);  
  connectToWifi();
  
  // Periodically read and send BME data:
  bmeTimerID = timer.setInterval(1000 * bmeInterval, wsSendBME);
}

//======================================
// LOOP
//======================================
void loop() {
  timer.run();
  webSocket.loop();  
}