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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("\n Starting");
  //int pinstatus;
  pinMode(wifimgr_PIN, INPUT);
  pinMode(apmode_PIN, INPUT);
  //pinstatus = digitalRead(TRIGGER_PIN);
  //Serial.println("PinStatus= ");
  //Serial.print(pinstatus);
  if (apmode_PIN){ //pins are pulled high as connected. the switch pulls it low
    if (WiFi.SSID()) {
      Serial.println("Using saved wifi credentials");
      //trying to fix connection in progress hanging
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();
      WiFi.begin();
      for (byte i = 0; i<5; i++){ //will try to connect 5 times in 10 seconds
        if(WiFi.waitForConnectResult() != WL_CONNECTED) {
          Serial.println("Connection Failed!");
          delay(2000);
        }
        else {
          Serial.println("Connection Success");
          break;
        }
      }
    }
    else {
      Serial.println("No wifi credentials saved");
      Serial.println("Fallback to Access Point Mode");
      //WIFI INIT AP mode
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
  	Serial.println(myIP);
    }
  }
  else {
    Serial.println("Access Point Mode selected");
    //WIFI INIT AP mode
  WiFi.softAP("WordClock");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  }

}


void loop() {

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


  // put your main code here, to run repeatedly:

}
