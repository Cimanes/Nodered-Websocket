//======================================================
// VARIABLES
//======================================================
struct Handler {            // Handler structure to manage handlers
  const char* topic;
  void (*handler)();
};
const Handler handlers[] = {
  { "gpio/", handleGPIO },
  { "debug", handleDebug },
  { "interval", handleInterval },
  { "espIP", handleIP},
  { "read", handleRead },
  { "reboot", handleReboot },
  #ifdef WIFI_MANAGER
    { "wifi", handleWifi },
  #endif
  #ifdef USE_OTA
    { "OTA", handleOTA }
  #endif
};
const byte handlerCount = sizeof(handlers) / sizeof(handlers[0]);
const uint16_t wifiReconnectTimer = 3000; // Timer to reconnect to Wifi after failed
const uint16_t wsReconnectTimer = 5000; // Timer to reconnect to WS after failed

//======================================================
// FUNCTIONS
//======================================================
void processMessage(uint8_t* wsMessage) {
  // check for error
  if (deserializeJson(jsonDoc, wsMessage)) {
    if (Debug) Serial.println(F("Invalid JSON"));
    return;
  }
  const char* topic = jsonDoc["topic"];
  if (!topic) {
    if (Debug) Serial.println(F("Invalid topic"));
    return;
  }
  const char* payload = jsonDoc["payload"];
  if (Debug)  Serial.printf_P(PSTR(">[WS] %s -> %s\n"), topic, payload);

  for (byte i = 0; i < handlerCount; i++) {
    if (strstr(topic, handlers[i].topic)) {
      handlers[i].handler();
      return;
    }
  }
}

void webSocketEvent(WStype_t type, uint8_t* wsMessage, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      if (Debug) Serial.println(F("[WS] Disconnected!"));
      break;
    case WStype_CONNECTED:
      if (Debug) Serial.printf_P(PSTR("[WS] Connected: %s\n"), wsMessage);
      timer.setTimeout(1000, wsSendBME); // Send BME data after 1s;
      break;
    case WStype_BIN:
      if (Debug) Serial.printf_P(PSTR(">[WS] bin: %u bytes\n"), length);
      break;
    case WStype_ERROR:
      if (Debug) Serial.printf_P(PSTR("[WS] err: %s\n"), wsMessage);
      break;
    case WStype_TEXT:
      processMessage(wsMessage);
      break;
    default: break;
  }
}

void initWebSocket() {
  if(Debug) Serial.printf_P(PSTR("Connecting W.S.: %s\n"), wsURL);
  webSocket.begin(hostIP, 1880, wsURL)            ; // Server address, port and URL.
  webSocket.onEvent(webSocketEvent)               ; // Event handler.
  webSocket.setReconnectInterval(wsReconnectTimer); // Periodically attempt reconnection if ws lost.  
}

//======================================================
// WIFI EVENT FUNCTIONS
//======================================================
void onwifiConnect(const WiFiEventStationModeGotIP& event) {
  if (Debug) { 
    Serial.println(WiFi.localIP());  
    Serial.println(F("initWiFi done"));
  }
  timer.setTimeout(2000, initWebSocket);
}

void onwifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  if (Debug)   Serial.println(F("Wifi disconnected."));
  webSocket.setReconnectInterval(-1)                  ; // Avoid websocket to attempt reconnection.
  webSocket.disconnect()                              ; // Close websocket.
  timer.setTimeout(wifiReconnectTimer, connectToWifi) ; // Attempt to reconnect to WiFi.
}