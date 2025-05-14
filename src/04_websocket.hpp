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
  readBME();        // Read data from BME280
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Iterate over keys[] and values[] to populate the JSON object "jsonDoc"
  // Resulting JSON object example: {"temp":231,"hum":360,"pres":9530}
  for (byte i = 0; i < numBmeKeys; i++)  { jsonDoc[bmeKeys[i]] = bmeValues[i]; }

  serializeJson(jsonDoc, wsMsg, sizeof(wsMsg)); // Convert jsonDoc to char array
  if (Debug) Serial.println(wsMsg);                 // Optional debug output

  // multiJsonPayload(bmeKeys, bmeValues, numBmeKeys);   // Create JSON payload with the readings
  webSocket.sendTXT(wsMsg);                       // Send the payload
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//======================================
// JSON topic + payload --> send via Websocket
//======================================
/******************************************************
 * Create a JSON object with topic and payload from a single key-value pair.
 * @param topic The key for the JSON object entry.
 * @param value The value corresponding to the key.
 */
void sendJsonInt(const char* key, uint16_t value) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Add key-value pair to the JSON object "jsonDoc"
  jsonDoc["topic"] = key;
  jsonDoc["payload"] = value;

  serializeJson(jsonDoc, wsMsg, sizeof(wsMsg)); // Convert jsonDoc to char array
  if (Debug) Serial.println(wsMsg);                 // Optional debug output
  webSocket.sendTXT(wsMsg);
}

void sendJsonString(const char* key, const char* value) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Add key-value pair to the JSON object "jsonDoc"
  jsonDoc["topic"] = key;
  jsonDoc["payload"] = value;

  serializeJson(jsonDoc, wsMsg, sizeof(wsMsg)); // Convert jsonDoc to char array
  if (Debug) Serial.println(wsMsg);             // Optional debug output
  webSocket.sendTXT(wsMsg);
}

//======================================
// HANDLERS: process incoming messages
//======================================
void handleHeater(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(HEATER_PIN, jsonDoc["payload"]);
  sendJsonInt("heater", digitalRead(HEATER_PIN));
}

void handleBoiler(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(BOILER_PIN, jsonDoc["payload"]);
  sendJsonInt("boiler", digitalRead(BOILER_PIN));
}

void handleLED(StaticJsonDocument<100>& jsonDoc) {
  digitalWrite(LED_BUILTIN, jsonDoc["payload"]);
  sendJsonInt("led", digitalRead(LED_BUILTIN));
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
  sendJsonInt("interval", bmeInterval);
}

void handleIP(StaticJsonDocument<100>& jsonDoc) {
  sendJsonString("espIP", esp_ip);
}

void handleDebug(StaticJsonDocument<100>& jsonDoc) {
  Debug = (int)jsonDoc["payload"];
  sendJsonInt("debug", Debug);
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
  { "espIP", handleIP},
  { "read", handleRead },
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
//     sendJsonInt("heater", digitalRead(HEATER_PIN));  
//   }
//   else if (strcmp(topic, "boiler") == 0) {
//     digitalWrite(BOILER_PIN, jsonDoc["payload"]);
//     sendJsonInt("boiler", digitalRead(BOILER_PIN));
//   }
//   else if (strcmp(topic, "led") == 0) {
//     digitalWrite(LED_BUILTIN, jsonDoc["payload"]);
//     sendJsonInt("led", digitalRead(LED_BUILTIN));
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
//     sendJsonInt("interval", bmeInterval);
//   }
//   else if (strcmp(topic, "debug") == 0) {
//     Debug = (int)jsonDoc["payload"];
//     sendJsonInt("debug", Debug);
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
  webSocket.begin(hostIP, 1880, "/ws/readings");
  // event handler
  webSocket.onEvent(webSocketEvent);
  // try every 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}