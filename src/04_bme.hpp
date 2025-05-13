//======================================
// LIBRARIES
//======================================
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//======================================
// VARIABLES
//======================================

Adafruit_BME280 bme;              // BME280 object
unsigned long BMETimerID;         // Timer ID
unsigned int bmeInterval = 30;    // Interval to publish values (s)

// Arrays with BME key-value pairs
const char* bmeKeys[3] = {"temp", "hum", "pres"};
uint16_t bmeValues[3];
const byte numBmeKeys = sizeof(bmeKeys) / sizeof(bmeKeys[0]);

//======================================
// FUNCTIONS
//======================================
// Initialize BME280
void initBME() {      
  if (!bme.begin(0x76)) {
    Serial.println(F("BME280 not found, check wiring!"));
    while (1);
  }
  if (Debug) Serial.println(F("BME280 initialized"));
}

// Read data from BME280, round to 1 decimal
void readBME() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  // Define values with *10 factor (round to one decimal)
  // Pressure is converted to mbar and then applied *10 factor
  bmeValues[0] = round(bme.readTemperature() * 10);
  bmeValues[1] = round(bme.readHumidity() * 10);
  bmeValues[2] = round(bme.readPressure() / 10);
  timer.setTimeout(100, []() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  });
  
}