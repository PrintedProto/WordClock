/*

https://github.com/platformio/framework-arduinoespressif8266/tree/master/libraries
https://github.com/esp8266/Arduino/tree/master/libraries
*/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>  //https://github.com/Links2004/arduinoWebSockets
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Hash.h>

// select wich pin will trigger the configuraton portal when set to LOW
#define wifimgr_PIN D7  //use wifimgr
#define apmode_PIN D6   //set ap mode
#define ota_PIN D2      //allow ota updates

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdater;

void connTOwifi(){
  if (WiFi.SSID()) {
    WiFi.mode(WIFI_STA);
    Serial.println("Using saved wifi credentials");
    //trying to fix connection in progress hanging
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
    WiFi.hostname("WordClock");//32 chars max
    WiFi.begin();
    for (byte i = 0; i<5; i++){ //will try to connect 5 times in 10 seconds
      if(WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed!");
        delay(2000);
      }
      else {
        Serial.println("Connection Success");
        Serial.print("LAN IP address: ");
        IPAddress myIP = WiFi.localIP();
        Serial.println(myIP);
        break;
      }
    }
  }
  else {
    Serial.println("No wifi credentials saved");
    WiFi.mode(WIFI_AP);
    Serial.println("Fallback to Access Point Mode");
    //WIFI INIT AP mode
    WiFi.softAP("WordClock-F");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
}

void allowOTA(){
  //ArduinoOTA code
  ArduinoOTA.setPort(8266);
  //ArduinoOTA.setHostname("WordClock-OTA");
  ArduinoOTA.setPassword("OTAadmin");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
      else // U_SPIFFS
      type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Accepting OTA updates");
    //OTA via http usage curl -u admin:admin -F "image=@firmware.bin" esp8266-webupdate.local/firmware
    httpUpdater.setup(&server, "/firmware", "WordClock-OTA", "OTAadmin");

}

#define USE_SERIAL Serial1

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            if(payload[0] == '#') {
                // we get RGB data

                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

              //  analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
              //  analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
              //  analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));
            }

            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

void setup() {
  Serial.begin(115200);
  pinMode(wifimgr_PIN, INPUT);
  pinMode(apmode_PIN, INPUT);
  pinMode(ota_PIN, INPUT);

  if (digitalRead(apmode_PIN)){ //pins are pulled high as connected. the switch pulls it low
    connTOwifi();
  }
  else {
    Serial.println("Access Point Mode selected");
    WiFi.mode(WIFI_AP);
    //WIFI INIT AP mode
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
  if (!digitalRead(ota_PIN)){
    allowOTA(); //allow ota updating
  }
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // start HTTP server
  server.begin();
}


void loop() {
  ArduinoOTA.handle();

  // is wifi config requested?
  if ( digitalRead(wifimgr_PIN) == LOW ) {
    //WiFiManager
    WiFiManager wifiManager; //Local intialization. Once its business is done, there is no need to keep it around
        if (!wifiManager.startConfigPortal("AP_Proto")) {
      Serial.println("failed to connect... timeout");
      delay(3000);
      ESP.reset();//reset and try again, or maybe put it to deep sleep
      delay(5000);
    }
    Serial.println("Connection Success");//if you get here you have connected to the WiFi
  }
  //end wifimgrsetupmode
  //AP mode selected?
  if (!digitalRead(apmode_PIN)){ //pins are pulled high as connected. the switch pulls it low
    //WIFI INIT AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
  else if ((WiFi.status() != WL_CONNECTED) && (WiFi.SSID())) {
    connTOwifi();
  }
  //AP mode selected?
  webSocket.loop();
  server.handleClient();
  // put your main code here, to run repeatedly:

}
//pio run --verbose -e nodemcu -t upload --upload-port 192.168
