//======================================
// VARIABLES
//======================================
#define HEATER_PIN 13
#define BOILER_PIN 15

//======================================
// FUNCTIONS
//======================================
void initConsole() {
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(BOILER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW); 
  digitalWrite(BOILER_PIN, LOW);
}