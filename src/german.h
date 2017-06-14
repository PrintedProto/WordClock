#ifndef _german_h
#define _german_h

#include "Adafruit_NeoMatrix.h"


#define wordLEDS 121
//#define minutePIN 2
#define minuteLEDS 4

class German : public Adafruit_NeoMatrix {

public:
	German(byte wordPIN = 3);
	void ledsOff();
	void minutesOff();
	void writeWords(int Words[]);
	void writeMinutes(int Minute);
	void displayWords();
	void displayMinutes();
	void getTime();
	void Word_Init();

protected:
	byte  hour, minute, second;
	static const byte wordPositions[][8];
	static const byte minutePositions[][5];
	static const byte colours[][3];
	static const char* colourName[9];
	byte LEDbrightness, maxBrightness, rValue, gValue, bValue, fadingDelay, fadingStep;
	boolean wordFading, minuteFading;

private:



};

#endif // !1
