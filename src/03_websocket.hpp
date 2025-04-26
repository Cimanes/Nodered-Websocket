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
  void (*handler)(JSONVar&);
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

void sendJsonPayload(const char* topic, unsigned int value) {
  JsonTopicPayload(topic, value);
  webSocket.sendTXT(wsPayload);
}

//======================================
// HANDLERS: process incoming messages
//======================================
void handleHeater(JSONVar& json) {
  digitalWrite(HEATER_PIN, json["payload"]);
  sendJsonPayload("heater", (bool)digitalRead(HEATER_PIN));
}

void handleBoiler(JSONVar& json) {
  digitalWrite(BOILER_PIN, json["payload"]);
  sendJsonPayload("boiler", (bool)digitalRead(BOILER_PIN));
}

void handleLED(JSONVar& json) {
  digitalWrite(LED_BUILTIN, json["payload"]);
  sendJsonPayload("led", (bool)digitalRead(LED_BUILTIN));
}

void handleRead(JSONVar& json) {
  timer.deleteTimer(BMETimerID);
  readAndSendBME();
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
}

void handleInterval(JSONVar& json) {
  bmeInterval = (int)json["payload"];
  timer.deleteTimer(BMETimerID);
  readAndSendBME();
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
  sendJsonPayload("interval", bmeInterval);
}

void handleDebug(JSONVar& json) {
  espDebug = (int)json["payload"];
  sendJsonPayload("debug", espDebug);
}

void handleReboot(JSONVar&) {
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
const int handlerCount = sizeof(handlers) / sizeof(handlers[0]);

void processMessage(uint8_t* wsMessage) {
  JSONVar json = JSON.parse((char*)wsMessage);
  const char* topic = json["topic"];
  if (!topic) return;
  for (int i = 0; i < handlerCount; i++) {
    if (strcmp(topic, handlers[i].topic) == 0) {
      handlers[i].handler(json);
      return;
    }
  }
}

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