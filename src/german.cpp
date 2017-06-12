// Wenn es Probleme mit der DCF77 Library gibt, ggf. die myDCF77.h der Liquid Clock nehmen
// WS2812 best practices (https://learn.adafruit.com/adafruit-neopixel-uberguide/overview):
// - add a beefy capacitor (i.e. 470 µF or higher) at each end of an LED string
// - connect positive at the beginning and GND at the end of the string
// - add a 300 to 500 Ohm resistor on the data pin
// - make sure that both Arduino and the LED string share the same GND!
// Remarks:
// - when using long LED strings the voltage can drop significantly till the end of the string! Resulting in a different color!

unsigned int  hour=00, minute=4, second=00;

// Neopixel
#include <Adafruit_NeoPixel.h>
#define wordPIN 3
#define wordLEDS 121
//#define minutePIN 2
//#define minuteLEDS 4
Adafruit_NeoPixel wordPixels = Adafruit_NeoPixel(wordLEDS, wordPIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel minutePixels = Adafruit_NeoPixel(minuteLEDS, minutePIN, NEO_GRB + NEO_KHZ800);
//int colourChangePin = 6;
//int colourChange = 0;
//int colourCyclePin = 8;

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

const int wordPositions[][8] = {{5 , 1, 2, 4, 5, 6, 0 , 0 }, // 5 LEDs total, Position 0,1,2,3,4 in the string   => 0: ES IST
                                {4 , 8, 9, 10, 11, 0 , 0 , 0 }, // 4 LEDs total, Position 5,6,7,8 in the string     => 1: MFÜNF
                                {4 , 12, 13, 14, 15, 0 , 0 , 0 }, //                                                  => 2: MZEHN
                                {7 , 16, 17, 18, 19, 20, 21, 22}, //                                                  => 3: ZWANZIG
                                {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }, // Not used in west german                          => 4: DREI
                                {7 , 62, 63, 64, 65, 66, 67, 68}, //                                                  => 5: VIERTEL
                                {4 , 55, 56, 57, 58, 0 , 0 , 0 }, //                                                  => 6: NACH
                                {3 , 59, 60, 61, 0 , 0 , 0 , 0 }, //                                                  => 7: VOR
                                {4 , 51, 52, 53, 54, 0 , 0 , 0 }, //                                                  => 8: HALB
                                {5 , 46, 47, 48, 49, 50, 0 , 0 }, //                                                  => 9: ZWÖLF
                                {4 , 35, 36, 37, 38, 0 , 0 , 0 }, //                                                  => 10: ZWEI
                                {3 , 37, 38, 39, 0 , 0 , 0 , 0 }, //                                                  => 11: EIN
                                {4 , 37, 38, 39, 40, 0 , 0 , 0 }, //                                                  => 12: EINS
                                {6 , 40, 41, 42, 43, 44, 45, 0 }, //                                                  => 13: SIEBEN
                                {4 , 31, 32, 33, 34, 0 , 0 , 0 }, //                                                  => 14: HDREI
                                {4 , 27, 28, 29, 30, 0 , 0 , 0 }, //                                                  => 15: HFÜNF
                                {3 , 16, 17, 18, 0 , 0 , 0 , 0 }, //                                                  => 16: ELF
                                {4 , 19, 20, 21, 22, 0 , 0 , 0 }, //                                                  => 17: NEUN
                                {4 , 23, 24, 25, 26, 0 , 0 , 0 }, //                                                  => 18: HVIER
                                {4 , 12, 13, 14, 15, 0 , 0 , 0 }, //                                                  => 19: ACHT
                                {4 , 8 , 9 , 10, 11, 0 , 0 , 0 }, //                                                  => 20: HZEHN
                                {5 , 0 , 1 , 2 , 3 , 4 , 0 , 0 }, //                                                  => 21: SECHS
                                {3 , 5 , 6 , 7 , 0 , 0 , 0 , 0 } //                                                   => 22: UHR
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
int LEDbrightness = 50;                                  // base LED brightness, value between 1 - 255 (with 255 being the brightest)
int maxBrightness = 150;                                // limit the maximum brightness to lower stress on the LEDs
int IncBrightPin=7;
int RedBrightPin=9;
// Value of each colour, starting with White
int rValue = 255;
int gValue = 255;
int bValue = 255;

// To turn LED fading on (=true) or off (=false)
boolean wordFading = true;
boolean minuteFading = true;
int fadingDelay = 20;
int fadingStep = 1;


// For DS3231 RTC
//#define ENABLE_RTC
#ifdef ENABLE_RTC
#include <Wire.h>               //I2C library
#include "RtcDS3231.h"    //RTC library

RTC_DS3231 RTC;
#endif //ENABLE_RTC

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

  Serial.println("");
  Serial.print(F("ES IST "));

  // now we display the appropriate minute counter
  if ((minute>4) && (minute<10)) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    Serial.print(F("FUENF NACH "));
  }
  if ((minute>9) && (minute<15)) {
    // MZEHN;
    Words[Words[0]+1] = 2;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    Serial.print(F("ZEHN NACH "));
  }
  if ((minute>14) && (minute<20)) {
    // VIERTEL;
    Words[Words[0]+1] = 5;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    Serial.print(F("VIERTEL NACH "));
  }
  if ((minute>19) && (minute<25)) {
    // ZWANZIG;
    Words[Words[0]+1] = 3;
    // NACH;
    Words[Words[0]+2] = 6;
    Words[0] += 2;
    Serial.print(F("ZWANZIG NACH "));
  }
  if ((minute>24) && (minute<30)) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // VOR;
    Words[Words[0]+2] = 7;
    // HALB;
    Words[Words[0]+3] = 8;
    Words[0] += 3;
    Serial.print(F("FUENF VOR HALB "));
  }
  if ((minute>29) && (minute<35)) {
    // HALB;
    Words[Words[0]+1] = 8;
    Words[0] += 1;
    Serial.print(F("HALB "));
  }
  if ((minute>34) && (minute<40)) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // NACH;
    Words[Words[0]+2] = 6;
    // HALB;
    Words[Words[0]+3] = 8;
    Words[0] += 3;
    Serial.print(F("FUENF NACH HALB "));
  }
  if ((minute>39) && (minute<45)) {
    // ZWANZIG;
    Words[Words[0]+1] = 3;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    Serial.print(F("ZWANZIG VOR "));
  }
  if ((minute>44) && (minute<50)) {
    // VIERTEL;
    Words[Words[0]+1] = 5;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    Serial.print(F("VIERTEL VOR "));
  }
  if ((minute>49) && (minute<55)) {
    // MZEHN;
    Words[Words[0]+1] = 2;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    Serial.print(F("ZEHN VOR "));
  }
  if (minute>54) {
    // MFUENF;
    Words[Words[0]+1] = 1;
    // VOR;
    Words[Words[0]+2] = 7;
    Words[0] += 2;
    Serial.print(F("FUENF VOR "));
  }

  if (minute <5) {
    switch (hour) {
    case 0:
        // ZWOELF;
        Words[Words[0]+1] = 9;
        Words[0] += 1;
        Serial.print(F("ZWOELF "));
        break;
    case 1:
      // EIN;
      Words[Words[0]+1] = 11;
      Words[0] += 1;
      Serial.print(F("EIN "));
      break;
    case 2:
      // ZWEI;
      Words[Words[0]+1] = 10;
      Words[0] += 1;
      Serial.print(F("ZWEI "));
      break;
    case 3:
      // HDREI;
      Words[Words[0]+1] = 14;
      Words[0] += 1;
      Serial.print(F("DREI "));
      break;
    case 4:
      // HVIER;
      Words[Words[0]+1] = 18;
      Words[0] += 1;
      Serial.print(F("VIER "));
      break;
    case 5:
      // HFUENF;
      Words[Words[0]+1] = 15;
      Words[0] += 1;
      Serial.print(F("FUENF "));
      break;
    case 6:
      // SECHS;
      Words[Words[0]+1] = 21;
      Words[0] += 1;
      Serial.print(F("SECHS "));
      break;
    case 7:
      // SIEBEN;
      Words[Words[0]+1] = 13;
      Words[0] += 1;
      Serial.print(F("SIEBEN "));
      break;
    case 8:
      // ACHT;
      Words[Words[0]+1] = 19;
      Words[0] += 1;
      Serial.print(F("ACHT "));
      break;
    case 9:
      // NEUN;
      Words[Words[0]+1] = 17;
      Words[0] += 1;
      Serial.print(F("NEUN "));
      break;
    case 10:
      // HZEHN;
      Words[Words[0]+1] = 20;
      Words[0] += 1;
      Serial.print(F("ZEHN "));
      break;
    case 11:
      // ELF;
      Words[Words[0]+1] = 16;
      Words[0] += 1;
      Serial.print(F("ELF "));
      break;
    case 12:
      // ZWOELF;
      Words[Words[0]+1] = 9;
      Words[0] += 1;
      Serial.print(F("ZWOELF "));
      break;
    }
  // UHR;
  Words[Words[0]+1] = 22;
  Words[0] += 1;
  Serial.print(F("UHR "));
  } else if ((minute < 25) && (minute >4)) {
    switch (hour) {
      case 0:
        // ZWOELF;
        Words[Words[0]+1] = 9;
        Words[0] += 1;
        Serial.print(F("ZWOELF "));
        break;
      case 1:
        // EINS;
        Words[Words[0]+1] = 12;
        Words[0] += 1;
        Serial.print(F("EINS"));
        break;
      case 2:
        // ZWEI;
        Words[Words[0]+1] = 10;
        Words[0] += 1;
        Serial.print(F("ZWEI "));
        break;
      case 3:
        // HDREI;
        Words[Words[0]+1] = 14;
        Words[0] += 1;
        Serial.print(F("DREI "));
        break;
      case 4:
        // HVIER;
        Words[Words[0]+1] = 18;
        Words[0] += 1;
        Serial.print(F("VIER "));
        break;
      case 5:
        // HFUENF;
        Words[Words[0]+1] = 15;
        Words[0] += 1;
        Serial.print(F("FUENF "));
        break;
      case 6:
        // SECHS;
        Words[Words[0]+1] = 21;
        Words[0] += 1;
        Serial.print(F("SECHS "));
        break;
      case 7:
        // SIEBEN;
        Words[Words[0]+1] = 13;
        Words[0] += 1;
        Serial.print(F("SIEBEN "));
        break;
      case 8:
        // ACHT;
        Words[Words[0]+1] = 19;
        Words[0] += 1;
        Serial.print(F("ACHT "));
        break;
      case 9:
        // NEUN;
        Words[Words[0]+1] = 17;
        Words[0] += 1;
        Serial.print(F("NEUN "));
        break;
      case 10:
        // HZEHN;
        Words[Words[0]+1] = 20;
        Words[0] += 1;
        Serial.print(F("ZEHN "));
        break;
      case 11:
        // ELF;
        Words[Words[0]+1] = 16;
        Words[0] += 1;
        Serial.print(F("ELF "));
        break;
      case 12:
        // ZWOELF;
        Words[Words[0]+1] = 9;
        Words[0] += 1;
        Serial.print(F("ZWOELF "));
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
          Serial.print(F("EINS "));
          break;
        case 1:
          // ZWEI;
          Words[Words[0]+1] = 10;
          Words[0] += 1;
          Serial.print(F("ZWEI "));
          break;
        case 2:
          // HDREI;
          Words[Words[0]+1] = 14;
          Words[0] += 1;
          Serial.print(F("DREI "));
          break;
        case 3:
          // HVIER;
          Words[Words[0]+1] = 18;
          Words[0] += 1;
          Serial.print(F("VIER "));
          break;
        case 4:
          // HFUENF;
          Words[Words[0]+1] = 15;
          Words[0] += 1;
          Serial.print(F("FUENF "));
          break;
        case 5:
          // SECHS;
          Words[Words[0]+1] = 21;
          Words[0] += 1;
          Serial.print(F("SECHS "));
          break;
        case 6:
          // SIEBEN;
          Words[Words[0]+1] = 13;
          Words[0] += 1;
          Serial.print(F("SIEBEN "));
          break;
        case 7:
          // ACHT;
          Words[Words[0]+1] = 19;
          Words[0] += 1;
          Serial.print(F("ACHT "));
          break;
        case 8:
          // NEUN;
          Words[Words[0]+1] = 17;
          Words[0] += 1;
          Serial.print(F("NEUN "));
          break;
        case 9:
          // HZEHN;
          Words[Words[0]+1] = 20;
          Words[0] += 1;
          Serial.print(F("ZEHN "));
          break;
        case 10:
          // ELF;
          Words[Words[0]+1] = 16;
          Words[0] += 1;
          Serial.print(F("ELF "));
          break;
        case 11:
          // ZWOELF;
          Words[Words[0]+1] = 9;
          Words[0] += 1;
          Serial.print(F("ZWOELF "));
          break;
        case 12:
          // EINS;
          Words[Words[0]+1] = 12;
          Words[0] += 1;
          Serial.print(F("EINS "));
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

void setup()
{
  // initialise the hardware
  // initialize the appropriate pins as outputs:

  // NOTE acc. to arduino.cc: If the pin isn't connected to anything, digitalRead() can return either HIGH or LOW (and this can change randomly).
  // This is also prevented by using the internal pullup!
  pinMode(IncBrightPin, INPUT_PULLUP);
  pinMode(RedBrightPin, INPUT_PULLUP);
  pinMode(colourChangePin, INPUT_PULLUP);
  pinMode(colourCyclePin, INPUT_PULLUP);

  // Initialize the LEDs
  wordPixels.setBrightness(LEDbrightness);
  minutePixels.setBrightness(LEDbrightness);
  wordPixels.begin();
  wordPixels.show();
  minutePixels.begin();
  minutePixels.show();

  Serial.begin(9600);
  Serial.println(F("Starting clock"));

/*
  // For RTC
  // To power the RTC from Pins A2 and A3
  pinMode(RTC_VCC, OUTPUT);
  digitalWrite(RTC_VCC, HIGH);
//  pinMode(RTC_GND, OUTPUT);
//  digitalWrite(RTC_GND, LOW);

  Wire.begin();
  RTC.begin();
  if (RTC.lostPower()) {
    Serial.println("RTC lost power - readjusting.");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  } else {
    Serial.println("RTC did not lose power - no need to set time.");
  }

  DateTime now = RTC.now();
  Serial.print(F("From RTC: "));
  Serial.print(now.hour(), DEC);
  Serial.print(F(" h, "));
  Serial.print(now.minute(), DEC);
  Serial.print(F(" min, "));
  Serial.print(now.second(), DEC);
  Serial.print(F(" s ; "));
  Serial.print(now.day(), DEC);
  Serial.print(F("."));
  Serial.print(now.month(), DEC);
  Serial.print(F("."));
  Serial.print(now.year(), DEC);
*/
  getTime();
  displayWords();
  displayMinutes();
}

void loop(void)
{
  unsigned int lastsec = second;

  getTime();

  // To refresh the display once [ second == 0 && abs(second-lastsec)!=0 ] every five minutes [ minute % 5 == 0  ; note: 0 modulo 5 = 0 ]
  if (minute % 5 ==0 && second == 0 && abs(second-lastsec)!=0){
//    Serial.print(hour);
//    Serial.print(F(" h, "));
//    Serial.print(minute);
//    Serial.print(F(" min, "));
//    Serial.print(second);
//    Serial.println(F(" s"));
    displayWords();
  }

  // To make use of the four minute LEDs
  if (second == 0 && abs(second-lastsec)!=0 && minute % 5 != 0){
    displayMinutes();
  }

  if (digitalRead(IncBrightPin) == LOW)
  {
    if ((LEDbrightness+10) <= maxBrightness)
    {
        LEDbrightness += 10;
        Serial.println("");
        Serial.print(F("LED brightness set to: "));
        Serial.print(LEDbrightness);
    }
    else{
        LEDbrightness = maxBrightness;
        Serial.println("");
        Serial.print(F("LED brightness set to: "));
        Serial.print(LEDbrightness);
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


  if (digitalRead(RedBrightPin) == LOW)
  {
    if ((LEDbrightness) >= 30)
    {
        LEDbrightness -= 10;
        Serial.println("");
        Serial.print(F("LED brightness set to: "));
        Serial.print(LEDbrightness);
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
    } else { colourChange = 0; };

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
