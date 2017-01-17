



#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// select wich pin will trigger the configuraton portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared
// with the bootloader, so always set them HIGH at power-up
#define wifimgr_PIN D7
#define apmode_PIN D6
#define ota_PIN D2

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
  ArduinoOTA.setHostname("WordClock-OTA");
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
  if (!digitalRead(apmode_PIN)){ //pins are pulled high as connected. the switch pulls it low
    //WIFI INIT AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
  else if (WiFi.status() != WL_CONNECTED) {
    connTOwifi();
  }


  // put your main code here, to run repeatedly:

}
