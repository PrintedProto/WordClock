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


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("\n Starting");
  //int pinstatus;
  pinMode(wifimgr_PIN, INPUT);
  //pinstatus = digitalRead(TRIGGER_PIN);
  //Serial.println("PinStatus= ");
  //Serial.print(pinstatus);
}


void loop() {

  // is wifi config requested?
  if ( digitalRead(wifimgr_PIN) == LOW ) {
    //WiFiManager
    WiFiManager wifiManager; //Local intialization. Once its business is done, there is no need to keep it around
    //wifiManager.resetSettings();//reset settings - for testing
    //wifiManager.setTimeout(120); //sets timeout until configuration portal gets turned off useful to make it all retry or go to sleep in seconds
    //WiFi.mode(WIFI_STA); //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1

    if (!wifiManager.startConfigPortal("AP_Proto")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.reset();//reset and try again, or maybe put it to deep sleep
      delay(5000);
    }

    Serial.println("connected...yeey :)");//if you get here you have connected to the WiFi
  }
  //end wifimgrsetupmode


  // put your main code here, to run repeatedly:

}
