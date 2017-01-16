/*
***************************************************************************
*                                                                         *
* This program is free software; you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation; either version 2 of the License, or       *
* (at your option) any later version.                                     *
*                                                                         *
* This program is distributed in the hope that it will be useful,         *
* but WITHOUT ANY WARRANTY; without even the implied warranty of          *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
* GNU General Public License for more details.                            *
*                                                                         *
* You should have received a copy of the GNU General Public License       *
* along with this program; if not, write to the Free Software             *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,                   *
* MA  02111-1307  USA                                                     *
*                                                                         *
***************************************************************************
*/
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
#define wordLEDS 98
#define minutePIN 2
#define minuteLEDS 4
Adafruit_NeoPixel wordPixels = Adafruit_NeoPixel(wordLEDS, wordPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minutePixels = Adafruit_NeoPixel(minuteLEDS, minutePIN, NEO_GRB + NEO_KHZ800);
int colourChangePin = 6;
int colourChange = 0;
int colourCyclePin = 8;

// 87┌--> F L A H --> N E T <-- S I <-- T I        97  IT IS MTEN HALF
// 86└--  Y T N E W T --> R E T R A U Q       <--┐ 74  QUARTER TWENTY
// 63┌--> S E T U N I M <-- E V I F            --┘ 73  MFIVE MINUTES
// 62└--  O W T <-- E N O <-- O T <-- T S A P <--┐ 51  PAST TO HONE HTWO
// 38┌--> E V I F --> R U O F --> E E R H T    --┘ 50  HTHREE HFOUR HFIVE
// 37└--  T H G I E <-- N E V E S <-- X I S   <--┐ 25  SIX SEVEN EIGHT
// 12┌--> N E V E L E --> N E T --> E N I N    --┘ 24  NINE HTEN ELEVEN
// 11└--  K C O L C O <-- E V L E W T          <-- 0   TWELVE OCLOCK

const int wordPositions[][8] = {{4 , 94, 95, 96, 97, 0 , 0 , 0 }, // 4 LEDs total, Position 94,95,96,97 in the string => 0: IT IS
                               {4 , 70, 71, 72, 73, 0 , 0 , 0 }, // 4 LEDs total, Position 70,71,72,73 in the string => 1: MFIVE
                               {3 , 91, 92, 93, 0 , 0 , 0 , 0 }, //                                                  => 2: MTEN
                               {6 , 81, 82, 83, 84, 85, 86, 0 }, //                                                  => 3: TWENTY
                               {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }, // Not used in EN                                   => 4: UNUSED
                               {7 , 74, 75, 76, 77, 78, 79, 80}, //                                                  => 5: QUARTER
                               {4 , 51, 52, 53, 54, 0 , 0 , 0 }, //                                                  => 6: PAST
                               {2 , 55, 56, 0 , 0 , 0 , 0 , 0 }, //                                                  => 7: TO
                               {4 , 87, 88, 89, 90, 0 , 0 , 0 }, //                                                  => 8: HALF
                               {6 , 0 , 1 , 2 , 3 , 4 , 5 , 0 }, //                                                  => 9: TWELVE
                               {3 , 60, 61, 62, 0 , 0 , 0 , 0 }, //                                                  => 10: HTWO
                               {3 , 57, 58, 59, 0 , 0 , 0 , 0 }, //                                                  => 11: HONE
                               {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }, //                                                  => 12: UNUSED
                               {5 , 28, 29, 30, 31, 32, 0 , 0 }, //                                                  => 13: SEVEN
                               {5 , 46, 47, 48, 49, 50, 0 , 0 }, //                                                  => 14: HTHREE
                               {4 , 38, 39, 40, 41, 0 , 0 , 0 }, //                                                  => 15: HFIVE
                               {6 , 12, 13, 14, 15, 16, 17, 0 }, //                                                  => 16: ELEVEN
                               {4 , 21, 22, 23, 24, 0 , 0 , 0 }, //                                                  => 17: NINE
                               {4 , 42, 43, 44, 45, 0 , 0 , 0 }, //                                                  => 18: HFOUR
                               {5 , 33, 34, 35, 36, 37, 0 , 0 }, //                                                  => 19: EIGHT
                               {3 , 18, 19, 20, 0 , 0 , 0 , 0 }, //                                                  => 20: HTEN
                               {3 , 25, 26, 27, 0 , 0 , 0 , 0 }, //                                                  => 21: SIX
                               {6 , 6 , 7 , 8 , 9 , 10, 11, 0 } //                                                   => 22: OCLOCK
};

// MTWO  <--  MONE
//   |          ^
//   |          |
//   v          |
// MTHREE      MFOUR <-- 0

const int minutePositions[][5] = {{1 , 1 , 0 , 0 , 0},                                                            //  => MONE
                                 {2 , 1 , 2 , 0 , 0},                                                            //  => MONE, MTWO
                                 {3 , 1 , 2 , 3 , 0},                                                            //  => MONE, MTWO, MTHREE
                                 {4 , 0 , 1 , 2 , 3}                                                             //  => MONE, MTWO, MTHREE, MFOUR
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
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 RTC;
// For Arduino Uno
//int RTC_VCC = 17;
//int RTC_GND = 16;
int RTC_VCC = 5;

boolean summertime_EU(int year, byte month, byte day, byte hour, byte tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
 if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
 if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
 if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7)))
   return true;
 else
   return false;
}

void ledsOff() {
 if ( wordFading == true && minuteFading == true){
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
 Serial.print(F("IT IS "));

 // now we display the appropriate words
 if ((minute>4) && (minute<10)) {
   // MFIVE;
   Words[Words[0]+1] = 1;
   // PAST;
   Words[Words[0]+2] = 6;
   Words[0] += 2;
   Serial.print(F("FIVE PAST "));
 }
 if ((minute>9) && (minute<15)) {
   // MTEN;
   Words[Words[0]+1] = 2;
   // PAST;
   Words[Words[0]+2] = 6;
   Words[0] += 2;
   Serial.print(F("TEN PAST "));
 }
 if ((minute>14) && (minute<20)) {
   // QUARTER;
   Words[Words[0]+1] = 5;
   // PAST;
   Words[Words[0]+2] = 6;
   Words[0] += 2;
   Serial.print(F("QUARTER PAST "));
 }
 if ((minute>19) && (minute<25)) {
   // TWENTY;
   Words[Words[0]+1] = 3;
   // PAST;
   Words[Words[0]+2] = 6;
   Words[0] += 2;
   Serial.print(F("TWENTY PAST "));
 }
 if ((minute>24) && (minute<30)) {
   // MFIVE;
   Words[Words[0]+1] = 1;
   // TO;
   Words[Words[0]+2] = 7;
   // HALF;
   Words[Words[0]+3] = 8;
   Words[0] += 3;
   Serial.print(F("FIVE TO HALF "));
 }
 if ((minute>29) && (minute<35)) {
   // HALF;
   Words[Words[0]+1] = 8;
   Words[0] += 1;
   Serial.print(F("HALF "));
 }
 if ((minute>34) && (minute<40)) {
   // MFIVE;
   Words[Words[0]+1] = 1;
   // PAST;
   Words[Words[0]+2] = 6;
   // HALF;
   Words[Words[0]+3] = 8;
   Words[0] += 3;
   Serial.print(F("FIVE PAST HALF "));
 }
 if ((minute>39) && (minute<45)) {
   // TWENTY;
   Words[Words[0]+1] = 3;
   // TO;
   Words[Words[0]+2] = 7;
   Words[0] += 2;
   Serial.print(F("TWENTY TO "));
 }
 if ((minute>44) && (minute<50)) {
   // QUARTER;
   Words[Words[0]+1] = 5;
   // TO;
   Words[Words[0]+2] = 7;
   Words[0] += 2;
   Serial.print(F("QUARTER TO "));
 }
 if ((minute>49) && (minute<55)) {
   // MTEN;
   Words[Words[0]+1] = 2;
   // TO;
   Words[Words[0]+2] = 7;
   Words[0] += 2;
   Serial.print(F("TEN TO "));
 }
 if (minute>54) {
   // MFIVE;
   Words[Words[0]+1] = 1;
   // TO;
   Words[Words[0]+2] = 7;
   Words[0] += 2;
   Serial.print(F("FIVE TO "));
 }

 if (minute <5) {
   switch (hour) {
   case 0:
       // TWELVE;
       Words[Words[0]+1] = 9;
       Words[0] += 1;
       Serial.print(F("TWELVE "));
       break;
   case 1:
     // HONE;
     Words[Words[0]+1] = 11;
     Words[0] += 1;
     Serial.print(F("ONE "));
     break;
   case 2:
     // HTWO;
     Words[Words[0]+1] = 10;
     Words[0] += 1;
     Serial.print(F("TWO "));
     break;
   case 3:
     // HTHREE;
     Words[Words[0]+1] = 14;
     Words[0] += 1;
     Serial.print(F("THREE "));
     break;
   case 4:
     // HFOUR;
     Words[Words[0]+1] = 18;
     Words[0] += 1;
     Serial.print(F("FOUR "));
     break;
   case 5:
     // HFIVE;
     Words[Words[0]+1] = 15;
     Words[0] += 1;
     Serial.print(F("FIVE "));
     break;
   case 6:
     // SIX;
     Words[Words[0]+1] = 21;
     Words[0] += 1;
     Serial.print(F("SIX "));
     break;
   case 7:
     // SEVEN;
     Words[Words[0]+1] = 13;
     Words[0] += 1;
     Serial.print(F("SEVEN "));
     break;
   case 8:
     // EIGHT;
     Words[Words[0]+1] = 19;
     Words[0] += 1;
     Serial.print(F("EIGHT "));
     break;
   case 9:
     // NINE;
     Words[Words[0]+1] = 17;
     Words[0] += 1;
     Serial.print(F("NINE "));
     break;
   case 10:
     // HTEN;
     Words[Words[0]+1] = 20;
     Words[0] += 1;
     Serial.print(F("TEN "));
     break;
   case 11:
     // ELEVEN;
     Words[Words[0]+1] = 16;
     Words[0] += 1;
     Serial.print(F("ELEVEN "));
     break;
   case 12:
     // TWELVE;
     Words[Words[0]+1] = 9;
     Words[0] += 1;
     Serial.print(F("TWELVE "));
     break;
   }
 // OCLOCK;
 Words[Words[0]+1] = 22;
 Words[0] += 1;
 Serial.print(F("OCLOCK "));
 } else if ((minute < 25) && (minute >4)) {
   switch (hour) {
     case 0:
       // TWELVE;
       Words[Words[0]+1] = 9;
       Words[0] += 1;
       Serial.print(F("TWELVE "));
       break;
     case 1:
       // HONE;
       Words[Words[0]+1] = 12;
       Words[0] += 1;
       Serial.print(F("ONE"));
       break;
     case 2:
       // HTWO;
       Words[Words[0]+1] = 10;
       Words[0] += 1;
       Serial.print(F("TWO "));
       break;
     case 3:
       // HTHREE;
       Words[Words[0]+1] = 14;
       Words[0] += 1;
       Serial.print(F("THREE "));
       break;
     case 4:
       // HFOUR;
       Words[Words[0]+1] = 18;
       Words[0] += 1;
       Serial.print(F("FOUR "));
       break;
     case 5:
       // HFIVE;
       Words[Words[0]+1] = 15;
       Words[0] += 1;
       Serial.print(F("FIVE "));
       break;
     case 6:
       // SIX;
       Words[Words[0]+1] = 21;
       Words[0] += 1;
       Serial.print(F("SIX "));
       break;
     case 7:
       // SEVEN;
       Words[Words[0]+1] = 13;
       Words[0] += 1;
       Serial.print(F("SEVEN "));
       break;
     case 8:
       // EIGHT;
       Words[Words[0]+1] = 19;
       Words[0] += 1;
       Serial.print(F("EIGHT "));
       break;
     case 9:
       // NINE;
       Words[Words[0]+1] = 17;
       Words[0] += 1;
       Serial.print(F("NINE "));
       break;
     case 10:
       // HTEN;
       Words[Words[0]+1] = 20;
       Words[0] += 1;
       Serial.print(F("TEN "));
       break;
     case 11:
       // ELEVEN;
       Words[Words[0]+1] = 16;
       Words[0] += 1;
       Serial.print(F("ELEVEN "));
       break;
     case 12:
       // TWELVE;
       Words[Words[0]+1] = 9;
       Words[0] += 1;
       Serial.print(F("TWELVE "));
       break;
     }
   } else {
     // if we are greater than 24 minutes past the hour then display
     // the next hour, as we will be displaying a 'to' sign
     switch (hour) {
       case 0:
         // HONE;
         Words[Words[0]+1] = 12;
         Words[0] += 1;
         Serial.print(F("ONE "));
         break;
       case 1:
         // HTWO;
         Words[Words[0]+1] = 10;
         Words[0] += 1;
         Serial.print(F("TWO "));
         break;
       case 2:
         // HTHREE;
         Words[Words[0]+1] = 14;
         Words[0] += 1;
         Serial.print(F("THREE "));
         break;
       case 3:
         // HFOUR;
         Words[Words[0]+1] = 18;
         Words[0] += 1;
         Serial.print(F("FOUR "));
         break;
       case 4:
         // HFIVE;
         Words[Words[0]+1] = 15;
         Words[0] += 1;
         Serial.print(F("FIVE "));
         break;
       case 5:
         // SIX;
         Words[Words[0]+1] = 21;
         Words[0] += 1;
         Serial.print(F("SIX "));
         break;
       case 6:
         // SEVEN;
         Words[Words[0]+1] = 13;
         Words[0] += 1;
         Serial.print(F("SEVEN "));
         break;
       case 7:
         // EIGHT;
         Words[Words[0]+1] = 19;
         Words[0] += 1;
         Serial.print(F("EIGHT "));
         break;
       case 8:
         // NINE;
         Words[Words[0]+1] = 17;
         Words[0] += 1;
         Serial.print(F("NINE "));
         break;
       case 9:
         // HTEN;
         Words[Words[0]+1] = 20;
         Words[0] += 1;
         Serial.print(F("TEN "));
         break;
       case 10:
         // ELEVEN;
         Words[Words[0]+1] = 16;
         Words[0] += 1;
         Serial.print(F("ELEVEN "));
         break;
       case 11:
         // TWELVE;
         Words[Words[0]+1] = 9;
         Words[0] += 1;
         Serial.print(F("TWELVE "));
         break;
       case 12:
         // HONE;
         Words[Words[0]+1] = 12;
         Words[0] += 1;
         Serial.print(F("ONE "));
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
 DateTime now = RTC.now();

 hour = now.hour();
 minute = now.minute();
 second = now.second();

 if (summertime_EU(now.year(), now.month(), now.day(), hour, 0)){
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
