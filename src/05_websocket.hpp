//======================================
// LIBRARIES
//======================================
#include <WebSocketsClient.h>

//======================================
// VARIABLES
//======================================
WebSocketsClient webSocket; // Websocket object
const char* wsURL = "/ws";  // Websocket URL to connect

//======================================
// FUNCTIONS
//======================================
// Read BME values and send them via Websocket
void readAndSendBME() {
  readBME();        // Read data from BME280 --> update bmeValues
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  makeJsonArray(numBmeKeys, bmeKeys, bmeValues);
  webSocket.sendTXT(wsMsg);                       // Send the payload
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//======================================
// HANDLERS: process incoming messages
//======================================
void handleGPIO(StaticJsonDocument<100>& jsonDoc) {
  for (byte i = 0; i < gpioCount; i++) {
    if (strstr(jsonDoc["topic"], gpioPins[i].topic)) {
      digitalWrite(gpioPins[i].gpio, jsonDoc["payload"] == "1" ? HIGH : LOW);
      makeJsonInt(gpioPins[i].topic, digitalRead(gpioPins[i].gpio));
      webSocket.sendTXT(wsMsg);
      if (Debug) Serial.printf_P(PSTR("Pub. %s: %s\n"), gpioPins[i].topic, digitalRead(gpioPins[i].gpio) ? "1" : "0");
      return;
    }
  }
}

void handleRead(StaticJsonDocument<100>& jsonDoc) {
  timer.deleteTimer(BMETimerID);
  readAndSendBME();
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
}

void handleInterval(StaticJsonDocument<100>& jsonDoc) {
  bmeInterval = jsonDoc["payload"].as<int>();
  timer.deleteTimer(BMETimerID);
  readAndSendBME();
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
  makeJsonInt("interval", bmeInterval);
  webSocket.sendTXT(wsMsg);
}

void handleIP(StaticJsonDocument<100>& jsonDoc) {
  makeJsonString("espIP", esp_ip);
  webSocket.sendTXT(wsMsg);
}

void handleDebug(StaticJsonDocument<100>& jsonDoc) {
  Debug = jsonDoc["payload"].as<int>();
  makeJsonInt("debug", Debug);
  webSocket.sendTXT(wsMsg);
}

#ifdef WIFI_MANAGER
void handleWifi(StaticJsonDocument<100>& jsonDoc) {
  deleteFile(LittleFS, ssidPath);
  deleteFile(LittleFS, passPath);
  deleteFile(LittleFS, ipPath);
  deleteFile(LittleFS, routerPath);
  deleteFile(LittleFS, hostPath);
  makeJsonString("wifi", "");
  webSocket.sendTXT(wsMsg);
}
#endif

void handleReboot(StaticJsonDocument<100>&) {
  // reboot = true;
  if (Debug) Serial.println(F("Rebooting"));
  #if defined(ESP32)  
    timer.setTimeout(3000, []() { esp_restart(); } );
  #elif defined(ESP8266)
    timer.setTimeout(3000, []() { ESP.restart(); } );
  #endif
}

void handleOTA(StaticJsonDocument<100>&) {
  if(Debug) Serial.println(F("OTA requested"));
  webSocket.setReconnectInterval(-1);  // Stop auto-reconnect
  webSocket.disconnect();              // Clean disconnect
  startOTAServer();
}