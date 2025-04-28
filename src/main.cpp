//======================================
// LIBRARIES
//======================================
#include "00_globals.hpp"
#include "01_console.hpp"
#include "02_bme.hpp"
#include "03_websocket.hpp"

//======================================
// SETUP
//======================================
void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  
  Serial.begin(115200);
  Serial.print("\n");
  timer.setTimeout(500, initConsole);
  timer.setTimeout(1000, initBME);
  timer.setTimeout(1500, initWifi);
  timer.setTimeout(2000, initWebsocket);
  BMETimerID = timer.setInterval(1000 * bmeInterval, readAndSendBME);
}

//======================================
// LOOP
//======================================
void loop() {
  timer.run();
  webSocket.loop();  
}