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
// GLOBAL FUNCTIONS
//======================================
// Read BME values and send them via Websocket
void readAndSendBME() {
  readBME();                                          // Read data from BME280
  multiJsonPayload(bmeKeys, bmeValues, numBmeKeys);   // Create JSON payload with the readings
  webSocket.sendTXT(wsPayload);                       // Send the payload
}

void sendJsonPayload(const char* topic, uint16_t value) {
  JsonTopicPayload(topic, value);
  webSocket.sendTXT(wsPayload);
}

//======================================
// HANDLERS: process incoming messages
//======================================
void handleHeater(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(HEATER_PIN, jsonDoc["payload"]);
  sendJsonPayload("heater", digitalRead(HEATER_PIN));
}

void handleBoiler(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(BOILER_PIN, jsonDoc["payload"]);
  sendJsonPayload("boiler", digitalRead(BOILER_PIN));
}

void handleLED(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(LED_BUILTIN, jsonDoc["payload"]);
  sendJsonPayload("led", digitalRead(LED_BUILTIN));
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
  sendJsonPayload("interval", bmeInterval);
}

void handleDebug(StaticJsonDocument<100>& jsonDoc) {
  espDebug = (int)jsonDoc["payload"];
  sendJsonPayload("debug", espDebug);
}

void handleReboot(StaticJsonDocument<100>&) {
  ESP.restart();
}

const Handler handlers[] = {
  { "heater", handleHeater },
  { "boiler", handleBoiler },
  { "debug", handleDebug },
  { "led", handleLED },
  { "interval", handleInterval },
  { "read", handleRead },
  { "reboot", handleReboot }
};
const byte handlerCount = sizeof(handlers) / sizeof(handlers[0]);

void processMessage(uint8_t* wsMessage) {
  // check for error
  if (deserializeJson(jsonDoc, wsMessage)) {
    if (espDebug) Serial.println(F("Invalid JSON"));
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
//     if (espDebug) Serial.println(F("Invalid JSON"));
//     return;
//   }
//   const char* topic = jsonDoc["topic"];

//   if(strcmp(topic, "heater") == 0) {
//     digitalWrite(HEATER_PIN, jsonDoc["payload"]);
//     sendJsonPayload("heater", digitalRead(HEATER_PIN));  
//   }
//   else if (strcmp(topic, "boiler") == 0) {
//     digitalWrite(BOILER_PIN, jsonDoc["payload"]);
//     sendJsonPayload("boiler", digitalRead(BOILER_PIN));
//   }
//   else if (strcmp(topic, "led") == 0) {
//     digitalWrite(LED_BUILTIN, jsonDoc["payload"]);
//     sendJsonPayload("led", digitalRead(LED_BUILTIN));
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
//     sendJsonPayload("interval", bmeInterval);
//   }
//   else if (strcmp(topic, "debug") == 0) {
//     espDebug = (int)jsonDoc["payload"];
//     sendJsonPayload("debug", espDebug);
//   }
//   else if (strcmp(topic, "reboot") == 0) {
//     ESP.restart();
//   } 
// }

void webSocketEvent(WStype_t type, uint8_t* wsMessage, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      if (espDebug) Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      if (espDebug) Serial.printf("[WSc] Connected to url: %s\n", wsMessage);
      break;
    case WStype_BIN:
      if (espDebug) Serial.printf("[WSc] get binary message: %u bytes\n", length);
      break;
    case WStype_ERROR:
      if (espDebug) Serial.printf("[WSc] Error! %s\n", wsMessage);
      break;
    case WStype_TEXT:
      if (espDebug)  Serial.printf("[WSc] get text message: %s\n", wsMessage);
      processMessage(wsMessage);
      break;
    default: break;
  }
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println(F("WiFi Failed!"));
    return;
  }
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());
}

void initWebsocket() {
  // server address, port and URL
  webSocket.begin(LOCALHOST_IP, 1880, "/ws/readings");
  // event handler
  webSocket.onEvent(webSocketEvent);
  // try every 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  // webSocket.enableHeartbeat(15000, 3000, 2);
}