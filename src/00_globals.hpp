//======================================
// LIBRARIES
//======================================
#include <Arduino.h>
#include <SimpleTimer.h>
#include <ArduinoJson.h>

//======================================
// VARIABLES
//======================================
boolean Debug = true;             // Enable/disable debugging
boolean reboot = false;           // Reboot flag
SimpleTimer timer;                // SimpleTimer object
StaticJsonDocument<100> jsonDoc;  // Dummy JSON document
char wsPayload[100];              // Dummy JSON char array to send

#define HEATER_PIN 13             // Pin used for heater signal
#define BOILER_PIN 15             // Pin used for boiler signal

// #define useOTA                   // OPTIONAL: Use OTA
#define wifiManager               // OPTIONAL: Use wifiManager to set SSID, Password and IP

// =====================================
// Setup GPIO's
//======================================
void initGPIO() {
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(BOILER_PIN, OUTPUT);
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW); 
  digitalWrite(BOILER_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);  
}

//======================================
// JSON FUNCTIONS
//======================================
/******************************************************
 * Create a JSON payload from arrays of key-value pairs.
 * @param keys Array of keys for the JSON object.
 * @param values Array of values corresponding to the keys.
 * @param numKeys Number of key-value pairs.
//  */
// void multiJsonPayload(const char* keys[], uint16_t values[], byte numKeys) {
//   jsonDoc.clear();  // Clear the document to avoid leftover data

//   // Iterate over keys[] and values[] to populate the JSON object "jsonDoc"
//   for (byte i = 0; i < numKeys; i++)  { jsonDoc[keys[i]] = values[i]; }

//   serializeJson(jsonDoc, wsPayload, sizeof(wsPayload)); // Convert jsonDoc to char array
//   if (Debug) Serial.println(wsPayload);                 // Optional debug output
// }