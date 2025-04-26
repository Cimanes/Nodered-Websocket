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
unsigned int bmeInterval = 10;  // Interval to publish values (s)

// Arrays with BME key-value pairs
const char* bmeKeys[3] = {"temp", "hum", "pres"};
unsigned int  bmeValues[3];
const byte numBmeKeys = sizeof(bmeKeys) / sizeof(bmeKeys[0]);

//======================================
// FUNCTIONS
//======================================
void initBME() {      // Initialize BME280
  if (!bme.begin(0x76)) {
    Serial.println(F("BME280 not found, check wiring!"));
    while (1);
  }
  if (espDebug) Serial.println(F("BME280 initialized"));
}

void readBME() {  // Read data from BME280, round to 1 decimal
  // Define values with *10 factor (round to one decimal)
  // Pressure is converted to mbar and then applied *10 factor
  bmeValues[0] = round(bme.readTemperature() * 10);
  bmeValues[1] = round(bme.readHumidity() * 10);
  bmeValues[2] = round(bme.readPressure() / 10);
}

