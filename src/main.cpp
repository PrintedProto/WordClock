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
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Hash.h>
#include "JsonStreamingParser.h"
#include "JsonListener.h"
#include "ExampleParser.h"
#include "Adafruit_NeoPixel.h"
// For DS3231 RTC
#include <Wire.h>               //I2C library
#include "RtcDS3231.h"    //RTC library

RtcDS3231<TwoWire> RTC(Wire); //SDA D2 SCL D1
JsonStreamingParser parser;
ExampleListener listener;
//#include <FSbrowser.h>
//https://github.com/nailbuster/esp8266FTPServer

// select wich pin will trigger the configuraton portal when set to LOW
//#define wifimgr_PIN D7  //use wifimgr
//#define apmode_PIN D6   //set ap mode
//#define ota_PIN D2      //allow ota updates

const char apmodefile[] = "/mode/apmode.txt"; //spiffs file for ap mode selection (0-512)
volatile byte apmodeSelect; //0 means AP mode is selected

const char otamodefile[] = "/mode/otamode.txt"; //spiffs file for OTA mode selection (0-512)
volatile byte otamodeSelect; //0 means ota mode is selected

const char mgrmodefile[] = "/mode/mgrmode.txt"; //spiffs file for wifi mgr mode selection (0-512)
volatile byte mgrmodeSelect; //1 means wifimgr mode is selected

#define debugMSG Serial //comment out serial to disable serial messages

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdater;
File fsUploadFile;

unsigned int  hour=00, minute=4, second=00; //start led code, ends line 669

#define wordPIN D1
#define wordLEDS 110
#define minutePIN D2
#define minuteLEDS 4
Adafruit_NeoPixel wordPixels = Adafruit_NeoPixel(wordLEDS, wordPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minutePixels = Adafruit_NeoPixel(minuteLEDS, minutePIN, NEO_GRB + NEO_KHZ800);
int colourChangePin = 6;
int colourChange = 0;
int colourCyclePin = 8;

// 80┌--> F N Ü F <-- T S I <-- S E                88   ES IST MFÜNF
// 79└--  G I Z N A W Z --> N H E Z          <--┐ 69   MZEHN ZWANZIG
// 62┌--> L E T R E I V <-- (I E R D)         --┘ 68   DREI VIERTEL
// 61└--  R O V --> H C A N                  <--┐ 55   NACH VOR
// 46┌--> F L Ö W Z <-- B L A H               --┘ 54   HALB ZWÖLF
// 45└--  N E B E I <-- S <-- N I E <-- W Z  <--┐ 35   ZWEI EIN SIEBEN
// 27┌--> F N Ü F --> I E R D                 --┘ 34   HDREI HFÜNF
// 26└--  R E I V <-- N U E N <-- F L E      <--┐ 16   ELF NEUN HVIER
// 8 ┌--> N H E Z --> T H C A                 --┘ 15   ACHT HZEHN
// 7 └--  R H U  <-- S H C E S                <--   0   SECHS UHR

// 109-- F N Ü F <-- T S I <-- S E           <-- 99   ES IST MFÜNF          convert address to complete pixel matrix
// 98 --  G I Z N A W Z <-- N H E Z          <-- 88   MZEHN ZWANZIG
// 87 -- L E T R E I V <-- (I E R D)         <-- 77   DREI VIERTEL
// 76 --  R O V <-- H C A N                  <-- 66   NACH VOR
// 65 -- F L Ö W Z <-- B L A H               <-- 55   HALB ZWÖLF
// 54 --  N E B E I <-- S <-- N I E <-- W Z  <-- 44   ZWEI EIN SIEBEN
// 43 -- F N Ü F <-- I E R D                 <-- 33   HDREI HFÜNF
// 32 --  R E I V <-- N U E N <-- F L E      <-- 22   ELF NEUN HVIER
// 21 --   N H E Z <-- T H C A               <-- 11    ACHT HZEHN
// 10 --  R H U  <-- S H C E S               <--  0    SECHS UHR

const int wordPositions[][8] = {{5 , 99, 100, 102, 103, 104, 0 , 0 }, // 5 LEDs total, Position 0,1,2,3,4 in the string   => 0: ES IST
                                {4 , 106, 107, 108, 109, 0 , 0 , 0 }, // 4 LEDs total, Position 5,6,7,8 in the string     => 1: MFÜNF
                                {4 , 88, 89, 90, 91, 0 , 0 , 0 }, //                                                  => 2: MZEHN
                                {7 , 92, 93, 94, 95, 96, 97, 98}, //                                                  => 3: ZWANZIG
                                {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }, // Not used in west german                          => 4: DREI
                                {7 , 81, 82, 83, 84, 85, 86, 87}, //                                                  => 5: VIERTEL
                                {4 , 68, 69, 70, 71, 0 , 0 , 0 }, //                                                  => 6: NACH
                                {3 , 72, 73, 74, 0 , 0 , 0 , 0 }, //                                                  => 7: VOR
                                {4 , 55, 56, 57, 58, 0 , 0 , 0 }, //                                                  => 8: HALB
                                {5 , 60, 61, 62, 63, 64, 0 , 0 }, //                                                  => 9: ZWÖLF
                                {4 , 44, 45, 46, 47, 0 , 0 , 0 }, //                                                  => 10: ZWEI
                                {3 , 46, 47, 48, 0 , 0 , 0 , 0 }, //                                                  => 11: EIN
                                {4 , 46, 47, 48, 49, 0 , 0 , 0 }, //                                                  => 12: EINS
                                {6 , 49, 50, 51, 52, 53, 54, 0 }, //                                                  => 13: SIEBEN
                                {4 , 34, 35, 36, 37, 0 , 0 , 0 }, //                                                  => 14: HDREI
                                {4 , 40, 41, 42, 43, 0 , 0 , 0 }, //                                                  => 15: HFÜNF
                                {3 , 22, 23, 24, 0 , 0 , 0 , 0 }, //                                                  => 16: ELF
                                {4 , 25, 26, 27, 28, 0 , 0 , 0 }, //                                                  => 17: NEUN
                                {4 , 29, 30, 31, 32, 0 , 0 , 0 }, //                                                  => 18: HVIER
                                {4 , 12, 13, 14, 15, 0 , 0 , 0 }, //                                                  => 19: ACHT
                                {4 , 16, 17, 18, 19, 0 , 0 , 0 }, //                                                  => 20: HZEHN
                                {5 , 2 , 3 , 4 , 5 , 6 , 0 , 0 }, //                                                  => 21: SECHS
                                {3 , 9 , 10, 11, 0 , 0 , 0 , 0 } //                                                   => 22: UHR
};

// MZWEI  <-- MEINS
//   |          ^
//   |          |
//   v          |
// MDREI      MVIER <-- 0

const int minutePositions[][5] = {{1 , 1 , 0 , 0 , 0},                                                            //  => MEINS
                                  {2 , 1 , 2 , 0 , 0},                                                            //  => MEINS, MZWEI
                                  {3 , 1 , 2 , 3 , 0},                                                            //  => MEINS, MZWEI, MDREI
                                  {4 , 0 , 1 , 2 , 3}                                                             //  => MEINS, MZWEI, MDREI
};


const int colours[][3] = {{255, 255, 255},                                                                        // WHITE
                          {255, 0  ,   0},                                                                        // RED
                          {0  , 255,   0},                                                                        // GREEN
                          {0  , 0  , 255},                                                                        // BLUE
                          {255, 135, 0  },                                                                        // ORANGE
                          {255, 255, 0  },                                                                        // YELLOW
                          {0  , 255, 255},                                                                        // CYAN
                          {128, 0  , 128},                                                                        // PURPLE
                          {139, 99 , 108}                                                                         // PINK
};

const char* colourName[9] = {"WHITE","RED","GREEN","BLUE","ORANGE","YELLOW","CYAN","PURPLE","PINK"};

// To adjust the brightness
int LEDbrightness = 30;                                  // base LED brightness, value between 1 - 255 (with 255 being the brightest)
int maxBrightness = 200;                                // limit the maximum brightness to lower stress on the LEDs
int IncBrightPin=7;
int RedBrightPin=9;
// Value of each colour, starting with green
int rValue = 0;
int gValue = 255;
int bValue = 0;

// To turn LED fading on (=true) or off (=false)
boolean wordFading = true;
boolean minuteFading = true;
int fadingDelay = 20;
int fadingStep = 1;

boolean daylightSaving(){
  /*if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7)))
    return true;
  else
    return false;*/
}

void ledsOff() {
  if ( wordFading == true && minuteFading == true){
//    int step = LEDbrightness / 20;                // fade out in 20 steps, 100 ms apart
//    for ( int i = LEDbrightness ; i > 0 ; i-=step ) {
    for ( int i = LEDbrightness ; i > 0 ; i-=fadingStep ) {
      for ( int j = 0 ; j < wordLEDS ; j++ ){                 // we fade every LED ...
        if ( wordPixels.getPixelColor(j) != 0){               // that is not off
          wordPixels.setPixelColor(j,i*rValue/255,i*gValue/255,i*bValue/255);
        }
      }
      for ( int j = 0 ; j < minuteLEDS ; j++ ){
        minutePixels.setPixelColor(j,i*rValue/255,i*gValue/255,i*bValue/255);   // here we know that each LED is ON
      }
      wordPixels.show();
      minutePixels.show();
      delay(fadingDelay);
    }
  }

  if ( wordFading == true && minuteFading == false){
//    int step = LEDbrightness / 20;                // fade out in 20 steps, 100 ms apart
//    for ( int i = LEDbrightness ; i > 0 ; i-=step ) {
    for ( int i = LEDbrightness ; i > 0 ; i-=fadingStep ) {
      for ( int j = 0 ; j < wordLEDS ; j++ ){                 // we fade every LED ...
        if ( wordPixels.getPixelColor(j) != 0){               // that is not off
          wordPixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
        }
      }
      wordPixels.show();
      delay(fadingDelay);
    }
  }

  if ( wordFading == false && minuteFading == true){
//    int step = LEDbrightness / 20;                // fade out in 20 steps, 100 ms apart
//    for ( int i = LEDbrightness ; i > 0 ; i-=step ) {
    for ( int i = LEDbrightness ; i > 0 ; i-=fadingStep ) {
      for ( int j = 0 ; j < minuteLEDS ; j++ ){
        minutePixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);   // here we know that each LED is ON
      }
      minutePixels.show();
      delay(fadingDelay);
    }
  }

  // make sure that they are really off and in case both wordFading and minuteFading is false
  for ( int j = 0 ; j < wordLEDS ; j++ ){
    wordPixels.setPixelColor(j,0,0,0);
  }
  for ( int j = 0 ; j < minuteLEDS ; j++ ){
    minutePixels.setPixelColor(j,0,0,0);
  }
  wordPixels.show();
  minutePixels.show();
}

void minutesOff() {
  if ( minuteFading == true){
//    int step = LEDbrightness / 20;                // fade out in 20 steps, 100 ms apart
//    for ( int i = LEDbrightness ; i > 0 ; i-=step ) {
    for ( int i = LEDbrightness ; i > 0 ; i-=fadingStep ) {
      for ( int j = 0 ; j < minuteLEDS ; j++ ){                 // we fade every LED ...
        if ( minutePixels.getPixelColor(j) != 0){               // that is not off
          minutePixels.setPixelColor(j,i*rValue/255,i*gValue/255,i*bValue/255);
        }
      }
      minutePixels.show();
      delay(fadingDelay);
    }
  }

  // make sure that they are really off and in case minuteFading is false
  for ( int j = 0 ; j < minuteLEDS ; j++ ){
    minutePixels.setPixelColor(j,0,0,0);
  }
  minutePixels.show();
}

void writeWords(int Words[]) {
if ( wordFading == true) {
    for ( int i = 0 ; i <= LEDbrightness ; i+=fadingStep ){
      for ( int k = 1 ; k < Words[0]+1 ; k++ ) {
        for ( int j=1 ; j < wordPositions[Words[k]][0]+1 ; j++ ){
          wordPixels.setPixelColor(wordPositions[Words[k]][j],i*rValue/255,i*gValue/255,i*bValue/255);
        }
      }
      wordPixels.show();
      delay(fadingDelay);
    }
  } else {
    for ( int k = 1 ; k < Words[0]+1 ; k++ ) {
      for ( int j=1 ; j < wordPositions[Words[k]][0]+1 ; j++ ){
        wordPixels.setPixelColor(wordPositions[Words[k]][j],LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    wordPixels.show();
  }
}

void writeMinutes(int Minute){
  if ( minuteFading == true ) {
    for ( int i = 0 ; i <= LEDbrightness ; i+=fadingStep ){
      for ( int j=1 ; j < minutePositions[Minute][0]+1 ; j++ ){
        minutePixels.setPixelColor(minutePositions[Minute][j],i*rValue/255,i*gValue/255,i*bValue/255);
      }
      minutePixels.show();
      delay(fadingDelay);
    }
  } else {
    for ( int j=1 ; j < minutePositions[Minute][0]+1 ; j++ ){
      minutePixels.setPixelColor(minutePositions[Minute][j],LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
    }
    minutePixels.show();
  }
}

void displayWords(){
  // start by clearing the display to a known state
  ledsOff();
  int Words[6] = {1, 0, 0, 0, 0, 0};

  //Serial.println("");
  //Serial.print(F("ES IST "));

  // now we display the appropriate minute counter
  if ((minute>4) && (minute<10)) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    //Serial.print(F("FUENF NACH "));
  }
  if ((minute>9) && (minute<15)) {
    // MZEHN;
    Words[Words[0]+1] = 2;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    //Serial.print(F("ZEHN NACH "));
  }
  if ((minute>14) && (minute<20)) {
    // VIERTEL;
    Words[Words[0]+1] = 5;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    //Serial.print(F("VIERTEL NACH "));
  }
  if ((minute>19) && (minute<25)) {
    // ZWANZIG;
    Words[Words[0]+1] = 3;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    //Serial.print(F("ZWANZIG NACH "));
  }
  if ((minute>24) && (minute<30)) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // VOR;
    Words[Words[0]+2] = 7;
    // HALB;
    Words[Words[0]+3] = 8;
    Words[0] += 3;
    //Serial.print(F("FUENF VOR HALB "));
  }
  if ((minute>29) && (minute<35)) {
    // HALB;
    Words[Words[0]+1] = 8;
    Words[0] += 1;
    //Serial.print(F("HALB "));
  }
  if ((minute>34) && (minute<40)) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // NACH;
    Words[Words[0]+2] = 6;
    // HALB;
    Words[Words[0]+3] = 8;
    Words[0] += 3;
    //Serial.print(F("FUENF NACH HALB "));
  }
  if ((minute>39) && (minute<45)) {
    // ZWANZIG;
    Words[Words[0]+1] = 3;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    //Serial.print(F("ZWANZIG VOR "));
  }
  if ((minute>44) && (minute<50)) {
    // VIERTEL;
    Words[Words[0]+1] = 5;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    //Serial.print(F("VIERTEL VOR "));
  }
  if ((minute>49) && (minute<55)) {
    // MZEHN;
    Words[Words[0]+1] = 2;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    //Serial.print(F("ZEHN VOR "));
  }
  if (minute>54) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    //Serial.print(F("FUENF VOR "));
  }

  if (minute <5) {
    switch (hour) {
    case 0:
        // ZWOELF;
        Words[Words[0]+1] = 9;
        Words[0] += 1;
        //Serial.print(F("ZWOELF "));
        break;
    case 1:
      // EIN;
      Words[Words[0]+1] = 11;
      Words[0] += 1;
      //Serial.print(F("EIN "));
      break;
    case 2:
      // ZWEI;
      Words[Words[0]+1] = 10;
      Words[0] += 1;
      //Serial.print(F("ZWEI "));
      break;
    case 3:
      // HDREI;
      Words[Words[0]+1] = 14;
      Words[0] += 1;
      //Serial.print(F("DREI "));
      break;
    case 4:
      // HVIER;
      Words[Words[0]+1] = 18;
      Words[0] += 1;
      //Serial.print(F("VIER "));
      break;
    case 5:
      // HFUENF;
      Words[Words[0]+1] = 15;
      Words[0] += 1;
      //Serial.print(F("FUENF "));
      break;
    case 6:
      // SECHS;
      Words[Words[0]+1] = 21;
      Words[0] += 1;
      //Serial.print(F("SECHS "));
      break;
    case 7:
      // SIEBEN;
      Words[Words[0]+1] = 13;
      Words[0] += 1;
      //Serial.print(F("SIEBEN "));
      break;
    case 8:
      // ACHT;
      Words[Words[0]+1] = 19;
      Words[0] += 1;
      //Serial.print(F("ACHT "));
      break;
    case 9:
      // NEUN;
      Words[Words[0]+1] = 17;
      Words[0] += 1;
      //Serial.print(F("NEUN "));
      break;
    case 10:
      // HZEHN;
      Words[Words[0]+1] = 20;
      Words[0] += 1;
      //Serial.print(F("ZEHN "));
      break;
    case 11:
      // ELF;
      Words[Words[0]+1] = 16;
      Words[0] += 1;
      //Serial.print(F("ELF "));
      break;
    case 12:
      // ZWOELF;
      Words[Words[0]+1] = 9;
      Words[0] += 1;
      //Serial.print(F("ZWOELF "));
      break;
    }
  // UHR;
  Words[Words[0]+1] = 22;
  Words[0] += 1;
  //Serial.print(F("UHR "));
  } else if ((minute < 25) && (minute >4)) {
    switch (hour) {
      case 0:
        // ZWOELF;
        Words[Words[0]+1] = 9;
        Words[0] += 1;
        //Serial.print(F("ZWOELF "));
        break;
      case 1:
        // EINS;
        Words[Words[0]+1] = 12;
        Words[0] += 1;
        //Serial.print(F("EINS"));
        break;
      case 2:
        // ZWEI;
        Words[Words[0]+1] = 10;
        Words[0] += 1;
        //Serial.print(F("ZWEI "));
        break;
      case 3:
        // HDREI;
        Words[Words[0]+1] = 14;
        Words[0] += 1;
        //Serial.print(F("DREI "));
        break;
      case 4:
        // HVIER;
        Words[Words[0]+1] = 18;
        Words[0] += 1;
        //Serial.print(F("VIER "));
        break;
      case 5:
        // HFUENF;
        Words[Words[0]+1] = 15;
        Words[0] += 1;
        //Serial.print(F("FUENF "));
        break;
      case 6:
        // SECHS;
        Words[Words[0]+1] = 21;
        Words[0] += 1;
        //Serial.print(F("SECHS "));
        break;
      case 7:
        // SIEBEN;
        Words[Words[0]+1] = 13;
        Words[0] += 1;
        //Serial.print(F("SIEBEN "));
        break;
      case 8:
        // ACHT;
        Words[Words[0]+1] = 19;
        Words[0] += 1;
        //Serial.print(F("ACHT "));
        break;
      case 9:
        // NEUN;
        Words[Words[0]+1] = 17;
        Words[0] += 1;
        //Serial.print(F("NEUN "));
        break;
      case 10:
        // HZEHN;
        Words[Words[0]+1] = 20;
        Words[0] += 1;
        //Serial.print(F("ZEHN "));
        break;
      case 11:
        // ELF;
        Words[Words[0]+1] = 16;
        Words[0] += 1;
        //Serial.print(F("ELF "));
        break;
      case 12:
        // ZWOELF;
        Words[Words[0]+1] = 9;
        Words[0] += 1;
        //Serial.print(F("ZWOELF "));
        break;
      }
    } else {
      // if we are greater than 24 minutes past the hour then display
      // the next hour, as we will be displaying a 'to' sign
      switch (hour) {
        case 0:
          // EINS;
          Words[Words[0]+1] = 12;
          Words[0] += 1;
          //Serial.print(F("EINS "));
          break;
        case 1:
          // ZWEI;
          Words[Words[0]+1] = 10;
          Words[0] += 1;
          //Serial.print(F("ZWEI "));
          break;
        case 2:
          // HDREI;
          Words[Words[0]+1] = 14;
          Words[0] += 1;
          //Serial.print(F("DREI "));
          break;
        case 3:
          // HVIER;
          Words[Words[0]+1] = 18;
          Words[0] += 1;
          //Serial.print(F("VIER "));
          break;
        case 4:
          // HFUENF;
          Words[Words[0]+1] = 15;
          Words[0] += 1;
          //Serial.print(F("FUENF "));
          break;
        case 5:
          // SECHS;
          Words[Words[0]+1] = 21;
          Words[0] += 1;
          //Serial.print(F("SECHS "));
          break;
        case 6:
          // SIEBEN;
          Words[Words[0]+1] = 13;
          Words[0] += 1;
          //Serial.print(F("SIEBEN "));
          break;
        case 7:
          // ACHT;
          Words[Words[0]+1] = 19;
          Words[0] += 1;
          //Serial.print(F("ACHT "));
          break;
        case 8:
          // NEUN;
          Words[Words[0]+1] = 17;
          Words[0] += 1;
          //Serial.print(F("NEUN "));
          break;
        case 9:
          // HZEHN;
          Words[Words[0]+1] = 20;
          Words[0] += 1;
          //Serial.print(F("ZEHN "));
          break;
        case 10:
          // ELF;
          Words[Words[0]+1] = 16;
          Words[0] += 1;
          //Serial.print(F("ELF "));
          break;
        case 11:
          // ZWOELF;
          Words[Words[0]+1] = 9;
          Words[0] += 1;
          //Serial.print(F("ZWOELF "));
          break;
        case 12:
          // EINS;
          Words[Words[0]+1] = 12;
          Words[0] += 1;
          //Serial.print(F("EINS "));
          break;
      }
   }
   writeWords(Words);
}

void displayMinutes(){
   minutesOff();
   // In order to make use of the four minute LEDs
   if (minute % 5 == 1) {
    writeMinutes(0);
//    Serial.print(F("+1 "));
  }
  if (minute % 5 == 2) {
    writeMinutes(1);
//    Serial.print(F("+2 "));
  }
  if (minute % 5 == 3) {
    writeMinutes(2);
//    Serial.print(F("+3 "));
  }
  if (minute % 5 == 4) {
    writeMinutes(3);
//    Serial.print(F("+4"));
  }
}

void getTime(){
  /*DateTime now = RTC.now();*/

  //hour = now.hour();
  //minute = now.minute();
  //second = now.second();
  hour = 5;
  minute = 2;
  second = 2;

  if (daylightSaving()){
   hour += 1;
  }
  //   else{
  //     hour += 1;
  //   }

  if (hour > 12){
   hour -= 12;
  }
}

void connTOwifi(){
  if (WiFi.SSID()) {
    WiFi.mode(WIFI_STA);
    //debugMSG.println("Using saved wifi credentials");
    //trying to fix connection in progress hanging
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
    WiFi.hostname("WordClock");//32 chars max
    WiFi.begin();
    for (byte i = 0; i<5; i++){ //will try to connect 5 times in 10 seconds
      if(WiFi.waitForConnectResult() != WL_CONNECTED) {
        //debugMSG.println("Connection Failed!");
        delay(2000);
      }
      else {
        //debugMSG.println("Connection Success");
        //debugMSG.print("LAN IP address: ");
        IPAddress myIP = WiFi.localIP();
        //debugMSG.println(myIP);
        break;
      }
    }
  }
  else { //wifi manager requested
    WiFiManager wifiManager; //Local intialization. Once its business is done, there is no need to keep it around
        if (!wifiManager.startConfigPortal("AP_Wordclock")) {
      //debugMSG.println("failed to connect... timeout");
      delay(3000);
      ESP.reset();//reset and try again, or maybe put it to deep sleep
      delay(5000);
    }
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
      debugMSG.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      debugMSG.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      debugMSG.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      debugMSG.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) debugMSG.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) debugMSG.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) debugMSG.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) debugMSG.println("Receive Failed");
      else if (error == OTA_END_ERROR) debugMSG.println("End Failed");
    });
    ArduinoOTA.begin();
    debugMSG.println("Ready");
    debugMSG.print("IP address: ");
    debugMSG.println(WiFi.localIP());
    debugMSG.println("Accepting OTA updates");
    //OTA via http usage curl -u admin:admin -F "image=@firmware.bin" esp8266-webupdate.local/firmware
    httpUpdater.setup(&server, "/firmware", "WordClock-OTA", "OTAadmin");

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            //debugMSG.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                //debugMSG.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            //debugMSG.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            //debugMSG.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}


String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  //debugMSG.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if((server.uri() != "/edit") && (server.uri() != "/upload") ) return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    //debugMSG.print("handleFileUpload Name: "); //debugMSG.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    ////debugMSG.print("handleFileUpload Data: "); //debugMSG.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    //debugMSG.print("handleFileUpload Size: "); //debugMSG.println(upload.totalSize);
    server.send(200, "text/plain", "Success");
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  //debugMSG.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  //debugMSG.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
  //debugMSG.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
void handleJSON(){
  //Serial.println(String(ESP.getFreeHeap()));
  parser.setListener(&listener);
  // put your setup code here, to run once:
  char json[] = "{\"a\":3, \"b\":{\"c\":\"d\"}}";
  for (int i = 0; i < sizeof(json); i++) {
    parser.parse(json[i]);
  }
  //Serial.println(String(ESP.getFreeHeap()));
}
void initServer(){
  //SERVER INIT
    //list directory
    server.on("/list", HTTP_GET, handleFileList);
    /*
    //load editor
    server.on("/edit", HTTP_GET, [](){
      if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
    });
    //create file
    server.on("/edit", HTTP_PUT, handleFileCreate);
    //delete file
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    //first callback is called after the request has ended with all parsed arguments
    //second callback handles file uploads at that location
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
    */
    //called when the url is not defined here
    //use it to load content from SPIFFS
    server.onNotFound([](){
      if(!handleFileRead(server.uri()))
        server.send(404, "text/plain", "FileNotFound");
    });

    //get heap status, analog input value and all GPIO statuses in one json call
    server.on("/all", HTTP_GET, [](){
      String json = "{";
      json += "\"heap\":"+String(ESP.getFreeHeap());
      json += ", \"analog\":"+String(analogRead(A0));
      json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
      json += "}";
      server.send(200, "text/json", json);
      json = String();
  });
  server.on("/sendJSON", handleJSON);
  server.on("/upload", HTTP_GET, []() {
    const char* UploadHTML = R"(<html><body><form method='POST' action='' enctype='multipart/form-data'>
                  <input type='file' name='upload'>
                  <input type='submit' value="upload">
               </form>
         </body></html>)";
         server.send(200, "text/html", UploadHTML);
    //server.sendHeader("Connection", "close");
    //server.sendHeader("Access-Control-Allow-Origin", "*");
    //server.send(200, "text/html", serverIndex);
  });
  server.on("/upload", HTTP_POST, [](){ server.send(200, "text/plain", "uploading"); }, handleFileUpload);
  server.on("/", HTTP_GET, [](){
    if(!handleFileRead("/index.htm")) server.send(404, "text/plain", "PageNotFound");
  });
  server.on("/ledcolor", HTTP_GET, [](){
    if(!handleFileRead("/ledcolor.htm")) server.send(404, "text/plain", "PageNotFound");
  });
  server.on("/test", HTTP_GET, [](){
    if(!handleFileRead("/test.htm")) server.send(404, "text/plain", "PageNotFound");
  });
  server.on("/mode", HTTP_GET, [](){
    if(!handleFileRead("/mode.htm")) server.send(404, "text/plain", "PageNotFound");
  });
  server.on("/message", HTTP_GET, [](){
    if(!handleFileRead("/message.htm")) server.send(404, "text/plain", "PageNotFound");
  });

}

void checkConfigmode(){
  {
  if(!SPIFFS.exists(apmodefile)){ //check if apmode file exists, if not create it
    File f = SPIFFS.open(apmodefile, "w");
      if(f){//verifies that file opened successfully
    f.print(0); //0 means ap mode is selected, default config
    f.close();
    apmodeSelect = 0;
      }
      else{
        //file failed to open
      }
    }
  else{
    File f = SPIFFS.open(apmodefile, "r"); //file exists open as readonly
    if(f){//verifies that file opened successfully
      apmodeSelect = f.parseInt(); //parse first integer in file
      if(apmodeSelect || !apmodeSelect){ //if integer is one or zero file is good, close file, continue program
        f.close();
        }
      else{
        f.close(); //if integer is not one or zero something is wrong, close file
        f = SPIFFS.open(apmodefile, "w"); // create new file on top of broken file
        if(f){
        f.print(0); //set file to deafault config
        f.close();
        apmodeSelect = 0;
            }
            else{
              //file failed to open
            }
        }
      }
      else{
        //file failed to open
      }
    }
  }//apmodefile
  {
  if(!SPIFFS.exists(otamodefile)){ //check if otamode file exists, if not create it
    File f = SPIFFS.open(otamodefile, "w");
      if(f){//verifies that file opened successfully
    f.print(0); //0 means ota mode is selected, default config
    f.close();
    otamodeSelect = 0;
      }
      else{
        //file failed to open
      }
    }
  else{
    File f = SPIFFS.open(otamodefile, "r"); //file exists open as readonly
    if(f){//verifies that file opened successfully
      otamodeSelect = f.parseInt(); //parse first integer in file
      if(otamodeSelect || !otamodeSelect){ //if integer is one or zero file is good, close file, continue program
        f.close();
        }
      else{
        f.close(); //if integer is not one or zero something is wrong, close file
        f = SPIFFS.open(otamodefile, "w"); // create new file on top of broken file
        if(f){
        f.print(0); //set file to deafault config
        f.close();
        otamodeSelect = 0;
            }
            else{
              //file failed to open
            }
        }
      }
      else{
        //file failed to open
      }
    }
  }//otamodefile
  {
  if(!SPIFFS.exists(mgrmodefile)){ //check if mgrmode file exists, if not create it
    File f = SPIFFS.open(mgrmodefile, "w");
      if(f){//verifies that file opened successfully
    f.print(0); //1 means wifi mgr mode is selected, default config 0
    f.close();
    mgrmodeSelect = 0;
      }
      else{
        //file failed to open
      }
    }
  else{
    File f = SPIFFS.open(mgrmodefile, "r"); //file exists open as readonly
    if(f){//verifies that file opened successfully
      mgrmodeSelect = f.parseInt(); //parse first integer in file
      if(mgrmodeSelect || !mgrmodeSelect){ //if integer is one or zero file is good, close file, continue program
        f.close();
        }
      else{
        f.close(); //if integer is not one or zero something is wrong, close file
        f = SPIFFS.open(mgrmodefile, "w"); // create new file on top of broken file
        if(f){
        f.print(0); //set file to deafault config
        f.close();
        mgrmodeSelect = 0;
            }
            else{
              //file failed to open
            }
        }
      }
      else{
        //file failed to open
      }
    }
  }//mgrmodefile
}

void handleRoot() {
if(!handleFileRead("/index.htm")) server.send(404, "text/plain", "FileNotFound");
}

void setup() {
  //debugMSG.begin(115200);

  SPIFFS.begin();
  checkConfigmode();

  if (apmodeSelect && !mgrmodeSelect){
    connTOwifi();
  }
  else if (!apmodeSelect && !mgrmodeSelect){
    //debugMSG.println("Access Point Mode selected");
    WiFi.mode(WIFI_AP);
    //WIFI INIT AP mode
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    //debugMSG.print("AP IP address: ");
    //debugMSG.println(myIP);
  }
  else{ //means mgrmodeselect is set to true
    WiFiManager wifiManager; //Local intialization. Once its business is done, there is no need to keep it around
        if (!wifiManager.startConfigPortal("AP_Wordclock")) {
      //debugMSG.println("failed to connect... timeout");
      delay(3000);
      ESP.reset();//reset and try again, or maybe put it to deep sleep
      delay(5000);
    }
  }
  if (!otamodeSelect){ //false means ota mode is selected
    allowOTA(); //allow ota updating
  }
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // start HTTP server
  initServer();
  server.begin();
  MDNS.begin("wordclock");

  // Initialize the LEDs
  wordPixels.setBrightness(LEDbrightness);
  minutePixels.setBrightness(LEDbrightness);
  wordPixels.begin();
  wordPixels.show();
  minutePixels.begin();
  minutePixels.show();
  getTime();
  displayWords();
  displayMinutes();

}


void loop() {
  ArduinoOTA.handle();

  webSocket.loop();
  server.handleClient();

  // put your main code here, to run repeatedly:
  unsigned int lastsec = second;

  getTime();

  // To refresh the display once [ second == 0 && abs(second-lastsec)!=0 ] every five minutes [ minute % 5 == 0  ; note: 0 modulo 5 = 0 ]
  if (minute % 5 ==0 && second == 0 && abs(second-lastsec)!=0){
    displayWords();
  }

  // To make use of the four minute LEDs
  if (second == 0 && abs(second-lastsec)!=0 && minute % 5 != 0){
    displayMinutes();
  }

  if (digitalRead(IncBrightPin) == LOW){ //increase brightness
    if ((LEDbrightness+10) <= maxBrightness){
        LEDbrightness += 10;
    }
    else{
        LEDbrightness = maxBrightness;
    }
    // set the new brightness on all LEDs that are on
    for ( int j = 0 ; j < wordLEDS ; j++ ){                 // every LED...
      if ( wordPixels.getPixelColor(j) != 0){               // that is not off (has a colour)
        wordPixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    for ( int j = 0 ; j < minuteLEDS ; j++ ){
      if ( minutePixels.getPixelColor(j) != 0){
        minutePixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    wordPixels.show();
    minutePixels.show();
    delay(500);
  }


  if (digitalRead(RedBrightPin) == LOW){//decrease brightness
    if ((LEDbrightness) >= 30) {//minimum brightness
        LEDbrightness -= 10;
    }
    // set the new brightness on all LEDs that are on
    for ( int j = 0 ; j < wordLEDS ; j++ ){                 // every LED...
      if ( wordPixels.getPixelColor(j) != 0){               // that is not off (has a colour)
        wordPixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    for ( int j = 0 ; j < minuteLEDS ; j++ ){
      if ( minutePixels.getPixelColor(j) != 0){
        minutePixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    wordPixels.show();
    minutePixels.show();
    delay(500);
  }

  if (digitalRead(colourChangePin) == LOW){
    if(colourChange < 8 ){
      colourChange++;
    }
    else {
      colourChange = 0;
    }

    Serial.println("");
    Serial.print(F("Setting colour to: "));
    Serial.print(colourName[colourChange]);
    rValue = colours[colourChange][0];
    gValue = colours[colourChange][1];
    bValue = colours[colourChange][2];
    Serial.print(F("; rValue: "));
    Serial.print(rValue);
    Serial.print(F("; gValue: "));
    Serial.print(gValue);
    Serial.print(F("; bValue: "));
    Serial.print(bValue);

    // set the new colour on all LEDs that are on
    for ( int j = 0 ; j < wordLEDS ; j++ ){                 // every LED
      if ( wordPixels.getPixelColor(j) != 0){               // that is not off
        wordPixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    for ( int j = 0 ; j < minuteLEDS ; j++ ){
      if ( minutePixels.getPixelColor(j) != 0){
        minutePixels.setPixelColor(j,LEDbrightness*rValue/255,LEDbrightness*gValue/255,LEDbrightness*bValue/255);
      }
    }
    wordPixels.show();
    minutePixels.show();
    delay(500);
  }

  if (digitalRead(colourCyclePin) == LOW){
    Serial.println("");
    Serial.print(F("Cycling colour, not yet implemented"));
    delay(500);
  }
}
//pio run --verbose -e nodemcu -t upload --upload-port 192.168
