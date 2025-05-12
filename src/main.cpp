//======================================
// LIBRARIES
//======================================
#include "00_globals.hpp"
#include "01_FileSys.hpp"
#include "02_wifi.hpp"
#include "03_console.hpp"
#include "04_bme.hpp"
#include "05_websocket.hpp"
#include "06_reboot.hpp"

//======================================
// SETUP
//======================================
void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  
  Serial.begin(115200);
  Serial.print("\n");
  initReboot();
  initFS();
  #if defined(wifiManager)  // Initialize Wifi, optional use wifiManager 
    getWiFi();              // Get SSID, Password and IP from files
    if(!initWiFi()) {       // If SSID or Password were not stored, manage them and reboot
      defineWiFi();
      return; 
    }
  #else
    initWiFi();     // Initialize Wifi with hardcoded values
  #endif
  timer.setTimeout(500, initConsole);
  timer.setTimeout(1000, initBME);
  // timer.setTimeout(1500, initWiFi);
  timer.setTimeout(2000, initWebsocket);
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
}

//======================================
// LOOP
//======================================
void loop() {
  timer.run();
  webSocket.loop();  
}