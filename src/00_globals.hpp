//======================================
// LIBRARIES
//======================================
#include <Arduino.h>
#include <SimpleTimer.h>
#include <ArduinoJson.h>

//======================================
// VARIABLES
//======================================
// Hardcoded Wifi credentials
// #define Toledo      // OPTIONAL: Choose Wifi credentials [Cimanes, Toledo, travel...]
// #if defined(Cimanes)
//   const char* ssid = "Pepe_Cimanes";
//   const char* pass = "Cimanes7581" ;
// #elif defined(Toledo)
//   const char* ssid = "MIWIFI_HtR7" ;
//   const char* pass = "TdQTDf3H"    ;
// #elif defined(travel)
//   const char* ssid = "InRoomWiFi"  ;
//   const char* pass = ""            ;
// #endif

#define LOCALHOST_IP "192.168.1.133"    // Node-red server IP
#define wifiManager
boolean Debug = true;                   // Enable/disable debugging
boolean reboot = false;                 // Reboot flag
SimpleTimer timer;                      // SimpleTimer object
StaticJsonDocument<100> jsonDoc;        // Dummy JSON document
char wsPayload[100];                    // Dummy JSON char array to send

//======================================
// JSON FUNCTIONS
//======================================
/**
 * Create a JSON payload from arrays of key-value pairs.
 * @param keys Array of keys for the JSON object.
 * @param values Array of values corresponding to the keys.
 * @param numKeys Number of key-value pairs.
 */
void multiJsonPayload(const char* keys[], uint16_t values[], byte numKeys) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Iterate over keys[] and values[] to populate the JSON object "jsonDoc"
  for (byte i = 0; i < numKeys; i++)  { jsonDoc[keys[i]] = values[i]; }

  serializeJson(jsonDoc, wsPayload, sizeof(wsPayload)); // Convert jsonDoc to char array
  if (Debug) Serial.println(wsPayload);              // Optional debug output
}

/**
 * Create a JSON payload from a single key-value pair.
 * @param key The key for the JSON object entry.
 * @param value The value corresponding to the key.
 */
void JsonTopicPayload(const char* key, uint16_t value) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Add key-value pair to the JSON object "jsonDoc"
  jsonDoc["topic"] = key;
  jsonDoc["payload"] = value;

  serializeJson(jsonDoc, wsPayload, sizeof(wsPayload)); // Convert jsonDoc to char array
  if (Debug) Serial.println(wsPayload);              // Optional debug output
}