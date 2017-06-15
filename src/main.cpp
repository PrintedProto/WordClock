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

#define wordledpin D3

void setup() {
    // WiFi is started inside library
    SPIFFS.begin(); // Not really needed, checked inside library and started if needed
    ESPHTTPServer.begin(&SPIFFS);
    German word = German(wordledpin);
    word.Word_Init();
    /* add setup code here */

}

void loop() {
    /* add main program code here */

    // DO NOT REMOVE. Attend OTA update from Arduino IDE
    //ESPHTTPServer.handle();
}
