//======================================
// LIBRARIES
//======================================
#include <Arduino.h>
#include <SimpleTimer.h>

//======================================
// VARIABLES
//======================================
boolean Debug = true;             // Enable/disable debugging
boolean reboot = false;           // Reboot flag
SimpleTimer timer;                // SimpleTimer object
char wsMsg[100];                  // Dummy char array to send message via websocket

#define HEATER_PIN 13             // Pin used for heater signal
#define BOILER_PIN 15             // Pin used for boiler signal

#define wifiManager               // OPTIONAL: Use wifiManager to set SSID, Password and IP
// #define useOTA                   // OPTIONAL: Use OTA

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