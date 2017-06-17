#include <ESP8266WiFi.h>
#include "FS.h"
#include <WiFiClient.h>
#include "TimeLib.h"
//#include "NtpClientLib.h"
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "ESP8266mDNS.h"
#include "Ticker.h"
#include "ArduinoOTA.h"
#include "ArduinoJson.h"
#include "FSWebServerLib.h"
#include "Hash.h"
#include "german.h"

#define wordledpin D5
//Hardware i2c default is D1 -> SCL and D2 -> SDA on the wemos D1 mini
// DS3231 SDA --> SDA
// DS3231 SCL --> SCL
// DS3231 VCC --> 3.3v or 5v
// DS3231 GND --> GND

void setup() {
    // WiFi is started inside library
    SPIFFS.begin(); // Not really needed, checked inside library and started if needed
    ESPHTTPServer.begin(&SPIFFS);
    German word = German(wordledpin);
    ESPHTTPServer.Word_Init(&word);
    /* add setup code here */

}

void loop() {
    /* add main program code here */
    if(ESPHTTPServer.chkTk()){
      if(ESPHTTPServer.getTime()){
        ESPHTTPServer.showTime();
      }
    }

    // DO NOT REMOVE. Attend OTA update from Arduino IDE
    //ESPHTTPServer.handle();
}
