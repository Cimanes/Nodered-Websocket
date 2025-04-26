//======================================
// LIBRARIES
//======================================
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <SimpleTimer.h>
#include <Arduino_JSON.h> 

//======================================
// VARIABLES
//======================================
// Hardcoded Wifi credentials 
#define Toledo      // OPTIONAL: Choose Wifi credentials [Cimanes, Toledo, travel...]
#if defined(Cimanes)
  const char* ssid = "Pepe_Cimanes";
  const char* pass = "Cimanes7581" ;
#elif defined(Toledo)
  const char* ssid = "MIWIFI_HtR7" ;
  const char* pass = "TdQTDf3H"    ;
#elif defined(travel)
  const char* ssid = "InRoomWiFi"  ;
  const char* pass = ""            ;
#endif

#define LOCALHOST_IP "192.168.1.131"    // Node-red server IP

boolean espDebug = true;    // Enable/disable debugging
SimpleTimer timer;          // SimpleTimer object
char wsPayload[100];        // Final JSON string to send

//======================================
// JSON FUNCTIONS
//======================================
/**
 * Create a JSON payload from arrays of keys and values.
 * @param keys Array of keys for the JSON object.
 * @param values Array of values corresponding to the keys.
 * @param numKeys Number of key-value pairs.
 */
void multiJsonPayload(const char* keys[], unsigned int values[], byte numKeys) {
  JSONVar jsonObj;
  // Iterate over keys[] and values[] to populate the JSON object
  for (byte i = 0; i < numKeys; i++)    jsonObj[keys[i]] = values[i]; 
  // Convert the JSON object to a char array
  JSON.stringify(jsonObj).toCharArray(wsPayload, sizeof(wsPayload));
  if (espDebug) Serial.println(wsPayload);
}

/**
 * Create a JSON payload from a single key-value pair.
 * @param key The key for the JSON object entry.
 * @param value The value corresponding to the key.
 */
void JsonTopicPayload(const char* key, unsigned int value) {
  JSONVar jsonObj;
  jsonObj["topic"] = key;
  jsonObj["payload"] = value;
  JSON.stringify(jsonObj).toCharArray(wsPayload, sizeof(wsPayload));
  if (espDebug) Serial.println(wsPayload);
}