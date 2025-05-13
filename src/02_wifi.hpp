// =============================================
// LIBRARIES
// =============================================
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#if defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#ifdef useOTA
  #include <AsyncElegantOTA.h>
#endif

// =============================================
// VARIABLES
// =============================================
// Create AsyncWebServer object on port 80, a WebSocket object ("/wsConsole") and an Event Source ("/eventsBME"):
AsyncWebServer server(80)               ;   // Required for HTTP 
const unsigned int cleanTimer = 2000UL  ;   // Timer to periodically clean websocket


#ifdef wifiManager
  IPAddress hostIP;             // User entry for node-red server IP
#else 
#define hostIP "192.168.1.133"  // Had coded Node-red server IP
#endif
// Option: Hard coded IP address:-------------------------
// IPAddress hostIP(192, 168, 1, 133);

#if defined(wifiManager)
  // =============================================
  // Wifi Manager: set SSID, Password and IP address
  // =============================================
  // Search for parameter in HTTP POST request received from user
  const char* PARAM_INPUT_1 = "ssid"  ;
  const char* PARAM_INPUT_2 = "pass"  ;
  const char* PARAM_INPUT_3 = "ip"    ;
  const char* PARAM_INPUT_4 = "router";
  const char* PARAM_INPUT_5 = "host"  ;
  // const char* PARAM_INPUTS[5] = { "ssid", "pass", "ip", "router", "host" };

  //Variables to save values from HTML form
  char ssid[paramSize]  ;
  char pass[paramSize]  ;
  char esp_ip[paramSize]    ;
  char router[paramSize];
  char host[paramSize]  ;

  // File paths to save input values permanently
  const char* ssidPath = "/ssid.txt"    ;
  const char* passPath = "/pass.txt"    ;
  const char* ipPath = "/ip.txt"        ;
  const char* routerPath = "/router.txt";
  const char* hostPath = "/host.txt"    ;
  // const char* paramPaths[5] = { "/ssid.txt", "/pass.txt", "/ip.txt", "/router.txt", "/host.txt" };

  // Function to Initialize Wifi
  bool initWiFi() {
    if(strcmp(ssid, "") == 0 || strcmp(esp_ip, "") == 0 || strcmp(router, "") == 0 ){
      Serial.println(F("Undefined WiFi"));
      return false;
    }  
    
    // Option: required for static IP address-----------------
    IPAddress subnet(255, 255, 0, 0);

    // Option: Hard coded IP address:-------------------------
    // IPAddress localIP(192, 168, 1, 200); 
    // IPAddress gateway(192, 168, 1, 254);
    // IPAddress dns(192, 168, 1, 254);
    // IPAddress hostIP(192, 168, 1, 133);
    
    // Option: user entry IP address:-------------------------
    IPAddress localIP;
    IPAddress gateway;
    IPAddress dns;
    localIP.fromString(esp_ip);       // Option: 
    gateway.fromString(router);
    dns.fromString(router);
    hostIP.fromString(host);

    if (!WiFi.config(localIP, gateway, subnet, dns)){
      if (Debug) Serial.println(F("STA config Failed"));
      return false;
    }
    //------------------------------------------------------------------------

    WiFi.mode(WIFI_STA);   
    WiFi.begin(ssid, pass);

    Serial.print(F("Connecting .."));
    while (WiFi.status() != WL_CONNECTED) { 
      Serial.print('.'); delay(1000);
    }
    if (Debug) Serial.println(WiFi.localIP());
    // Serve files (JS, CSS and favicon) from LittleFS when requested by the root URL. 
    server.serveStatic("/", LittleFS, "/");
    #ifdef useOTA
      AsyncElegantOTA.begin(&server); // Start OTA
    #endif
    server.begin(); // Start the server.    
    Serial.println(F("initWiFi done"));
    return true;
  }

  void getWiFi() {
    fileToCharPtr(LittleFS, ssidPath, ssid)     ; // Search for stored SSID
    fileToCharPtr(LittleFS, passPath, pass)     ; // Search for stored Password
    fileToCharPtr(LittleFS, ipPath, esp_ip)         ; // Search for stored local IP
    fileToCharPtr(LittleFS, routerPath, router) ; // Search for stored router IP
    fileToCharPtr(LittleFS, hostPath, host)     ; // Search for stored host IP
  }

  // =============================================
  // Function to allow user to enter ssid and password
  // =============================================
  // @details This function is used to connect to an ESP Wi-Fi network with a given SSID and password. 
  //          It also starts a web server to allow the user to input these values.
  //          The values are stored in the LittleFS file system of the ESP. If the values are not defined, 
  //          the ESP will start a Wi-Fi network with the name "ESP-WIFI-MANAGER" and no password.
  //          The user can then connect to this network and open the web page at the IP address of the ESP (usually 192.168.4.1)
  //          to input the values. The ESP will then reboot and connect to the Wi-Fi network with the given values.
  void defineWiFi() {
    Serial.println(F("Setting AP")); 
    // Remove the password parameter (=NULL), if you want the AP (Access Point) to be open 
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print(F("AP local IP: "));
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            strncpy(ssid, p->value().c_str(), paramSize - 1);
            ssid[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.print(F("SSID: "));  Serial.println(ssid);
            writeFile(LittleFS, ssidPath, ssid);
          }
          // HTTP POST pass value
          else if (p->name() == PARAM_INPUT_2) {
            strncpy(pass, p->value().c_str(), paramSize - 1);
            pass[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.print(F("Password: "));  Serial.println(pass);
            writeFile(LittleFS, passPath, pass);
          }
          // HTTP POST esp_ip value
          else if (p->name() == PARAM_INPUT_3) {
            strncpy(esp_ip, p->value().c_str(), paramSize - 1);
            esp_ip[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.print(F("Local IP: "));  Serial.println(esp_ip);
            writeFile(LittleFS, ipPath, esp_ip);
          }
          // HTTP POST router IP value
          else if (p->name() == PARAM_INPUT_4) {
            strncpy(router, p->value().c_str(), paramSize - 1);
            router[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.print(F("Router IP: "));
            Serial.println(router);
            writeFile(LittleFS, routerPath, router);
          }
          else if (p->name() == PARAM_INPUT_5) {
            strncpy(host, p->value().c_str(), paramSize - 1);
            host[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.print(F("Host IP: "));
            Serial.println(host);
            writeFile(LittleFS, hostPath, host);
          }           
          if (Debug) Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str()); 
        }
      }
      request->send(200, "text/plain", "Done. ESP rebooting, connect to your router. ESP IP address: " + String(esp_ip));
      reboot = true;
    });
    
    // Serve files (JS, CSS and favicon) from LittleFS when requested by the root URL. 
    server.serveStatic("/", LittleFS, "/");
    server.begin(); // Start the server.
    Serial.println(F("defineWifi done"));
  }
#else
  // =============================================
  // Hardcoded Wifi credentials 
  // =============================================
  #define Toledo      // OPTIONAL: Choose Wifi credentials [Cimanes, Toledo, apartment]
  #if defined(Cimanes)
    const char ssid[] = "Pepe_Cimanes";
    const char pass[] = "Cimanes7581" ;
  #elif defined(Toledo)
    const char ssid[] = "MIWIFI_HtR7" ;
    const char pass[] = "TdQTDf3H"    ;
  #elif defined(apartment)
    const char ssid[] = "John-Rs-Foodhall_EXT" ;
    const char pass[] = "sive2017"    ;
  #endif

  // Function to Initialize Wifi
  void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    Serial.print(F("Connecting to WiFi .."));
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    // Serve files (JS, CSS and favicon) from LittleFS when requested by the root URL. 
    server.serveStatic("/", LittleFS, "/");
    #ifdef useOTA
      AsyncElegantOTA.begin(&server); // Start OTA
    #endif
    server.begin();                   // Start the server.
    Serial.println(WiFi.localIP());
  }
#endif
