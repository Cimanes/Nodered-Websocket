//======================================
// LIBRARIES
//======================================
#include <WebSocketsClient.h>

//======================================
// VARIABLES
//======================================
WebSocketsClient webSocket; // Websocket object
struct Handler {            // Handler structure to manage handlers
  const char* topic;
  void (*handler)(StaticJsonDocument<100>&);
};

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
void handleHeater(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(HEATER_PIN, jsonDoc["payload"]);
  makeJsonInt("heater", digitalRead(HEATER_PIN));
  webSocket.sendTXT(wsMsg);
}

void handleBoiler(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(BOILER_PIN, jsonDoc["payload"]);
  makeJsonInt("boiler", digitalRead(BOILER_PIN));
  webSocket.sendTXT(wsMsg);
}

void handleLED(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(LED_BUILTIN, jsonDoc["payload"]);
  makeJsonInt("led", digitalRead(LED_BUILTIN));
  webSocket.sendTXT(wsMsg);
}

void handleRead(StaticJsonDocument<100>& jsonDoc) {
  timer.deleteTimer(BMETimerID);
  readAndSendBME();
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
}

void handleInterval(StaticJsonDocument<100>& jsonDoc) {
  bmeInterval = (int)jsonDoc["payload"];
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
  Debug = (int)jsonDoc["payload"];
  makeJsonInt("debug", Debug);
  webSocket.sendTXT(wsMsg);
}

#ifdef wifiManager
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
  ESP.restart();
}

const Handler handlers[] = {
  { "heater", handleHeater },
  { "boiler", handleBoiler },
  { "debug", handleDebug },
  { "led", handleLED },
  { "interval", handleInterval },
  { "espIP", handleIP},
  { "read", handleRead },
  #ifdef wifiManager
  { "wifi", handleWifi },
  #endif
  { "reboot", handleReboot }
};
const byte handlerCount = sizeof(handlers) / sizeof(handlers[0]);

void processMessage(uint8_t* wsMessage) {
  // check for error
  if (deserializeJson(jsonDoc, wsMessage)) {
    if (Debug) Serial.println(F("Invalid JSON"));
    return;
  }
  const char* topic = jsonDoc["topic"];
  if (!topic) return;
  for (byte i = 0; i < handlerCount; i++) {
    if (strcmp(topic, handlers[i].topic) == 0) {
      handlers[i].handler(jsonDoc);
      return;
    }
  }
}

// void processMessage(uint8_t* wsMessage) {
//   if (deserializeJson(jsonDoc, wsMessage)) {
//     if (Debug) Serial.println(F("Invalid JSON"));
//     return;
//   }
//   const char* topic = jsonDoc["topic"];

//   if(strcmp(topic, "heater") == 0) {
//     digitalWrite(HEATER_PIN, jsonDoc["payload"]);
//     makeJsonInt("heater", digitalRead(HEATER_PIN));  
//   }
//   else if (strcmp(topic, "boiler") == 0) {
//     digitalWrite(BOILER_PIN, jsonDoc["payload"]);
//     makeJsonInt("boiler", digitalRead(BOILER_PIN));
//   }
//   else if (strcmp(topic, "led") == 0) {
//     digitalWrite(LED_BUILTIN, jsonDoc["payload"]);
//     makeJsonInt("led", digitalRead(LED_BUILTIN));
//   }
//   else if (strcmp(topic, "read") == 0) {
//     timer.deleteTimer(BMETimerID);
//     readAndSendBME();
//     BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
//   }
//   else if (strcmp(topic, "interval") == 0) {
//     bmeInterval = (int)jsonDoc["payload"];
//     timer.deleteTimer(BMETimerID);
//     readAndSendBME();
//     BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
//     makeJsonInt("interval", bmeInterval);
//   }
//   else if (strcmp(topic, "debug") == 0) {
//     Debug = (int)jsonDoc["payload"];
//     makeJsonInt("debug", Debug);
//   }
//   else if (strcmp(topic, "reboot") == 0) {
//     ESP.restart();
//   } 
// }

void webSocketEvent(WStype_t type, uint8_t* wsMessage, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      if (Debug) Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      if (Debug) Serial.printf("[WSc] Connected to url: %s\n", wsMessage);
      break;
    case WStype_BIN:
      if (Debug) Serial.printf("[WSc] get binary message: %u bytes\n", length);
      break;
    case WStype_ERROR:
      if (Debug) Serial.printf("[WSc] Error! %s\n", wsMessage);
      break;
    case WStype_TEXT:
      if (Debug)  Serial.printf("[WSc] get text message: %s\n", wsMessage);
      processMessage(wsMessage);
      break;
    default: break;
  }
}

void initWebsocket() {
  // server address, port and URL
  webSocket.begin(hostIP, 1880, "/ws");
  // event handler
  webSocket.onEvent(webSocketEvent);
  // try every 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}