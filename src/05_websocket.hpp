//======================================
// LIBRARIES
//======================================
#include <WebSocketsClient.h>

//======================================
// VARIABLES
//======================================
WebSocketsClient webSocket; // Websocket object
const char* wsURL = "/ws" ; // Websocket URL to connect

//======================================
// FUNCTIONS
//======================================
// Read BME values and send them via Websocket
void wsSendBME() {
  readBME();        // Read data from BME280 --> update bmeValues
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  makeJsonArray(numBmeKeys, bmeKeys, bmeValues);
  webSocket.sendTXT(wsMsg);                       // Send the payload
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//======================================
// HANDLERS: process incoming messages
//======================================
void handleGPIO() {
  for (byte i = 0; i < gpioCount; i++) {
    if (strstr(jsonDoc["topic"], gpioPins[i].topic)) {
      digitalWrite(gpioPins[i].gpio, jsonDoc["payload"] == "1" ? HIGH : LOW);
      makeJsonInt(gpioPins[i].topic, digitalRead(gpioPins[i].gpio));
      webSocket.sendTXT(wsMsg);
      return;
    }
  }
}

void handleRead() {
  timer.deleteTimer(bmeTimerID);
  wsSendBME();
  bmeTimerID = timer.setInterval(1000 * bmeInterval, wsSendBME);
}

void handleInterval() {
  bmeInterval = jsonDoc["payload"].as<int>();
  timer.deleteTimer(bmeTimerID);
  wsSendBME();
  bmeTimerID = timer.setInterval(1000 * bmeInterval, wsSendBME);
  makeJsonInt("interval", bmeInterval);
  webSocket.sendTXT(wsMsg);
}

void handleIP() {
  makeJsonString("espIP", esp_ip);
  webSocket.sendTXT(wsMsg);
}

void handleDebug() {
  Debug = jsonDoc["payload"].as<int>();
  makeJsonInt("debug", Debug);
  webSocket.sendTXT(wsMsg);
}

void handleReboot() {
  if (Debug) Serial.println(F("Rebooting"));
  #if defined(ESP32)  
    timer.setTimeout(3000, []() { esp_restart(); } );
  #elif defined(ESP8266)
    timer.setTimeout(3000, []() { ESP.restart(); } );
  #endif
}

#ifdef WIFI_MANAGER
  void handleWifi() {
    deleteFile(LittleFS, ssidPath)    ;
    deleteFile(LittleFS, passPath)    ;
    deleteFile(LittleFS, ipPath)      ;
    deleteFile(LittleFS, routerPath)  ;
    deleteFile(LittleFS, hostPath)    ;
    makeJsonString("wifi", "")        ;
    webSocket.sendTXT(wsMsg)          ;
  }
#endif

#ifdef USE_OTA
  void handleOTA() {
    if(Debug) Serial.println(F("OTA requested"));
    webSocket.setReconnectInterval(-1);  // Stop auto-reconnect
    webSocket.disconnect();              // Clean disconnect
    startOTAServer();
  }
#endif