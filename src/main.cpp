//======================================
// LIBRARIES
//======================================
#include "00_globals.hpp"
#include "01_JSON.hpp"
#include "02_FileSys.hpp"
#include "03_wifi.hpp"
#include "04_bme.hpp"
#include "05_websocket.hpp"
// #include "06_reboot.hpp"
#include "06_events.hpp"

//======================================
// SETUP
//======================================
void setup() {
  Serial.begin(115200);
  Serial.print("\n");
  initFS();                 // Initialize File System
  initGPIO();
  initBME();

  // Initialize wifi and MQTT
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onwifiDisconnect);
  wifiConnectHandler = WiFi.onStationModeGotIP(onwifiConnect);  
  connectToWifi();  
  // initWebsocket();

  // initReboot();             // Initialize Reboot
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
}

//======================================
// LOOP
//======================================
void loop() {
  timer.run();
  webSocket.loop();  
}