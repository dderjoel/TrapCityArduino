#include <FastLED.h>

#define NUM_LEDS 216
#define ROWS 9 //rows of LEDs
#define COLS NUM_LEDS/ROWS
CRGB leds[NUM_LEDS];

#define LEDSPIN 12
int analogPin = 0;
int analogThreasholdPin = 1;
#define THREASHOLD_DEFAULT 400

int strobePin = 10;
int resetPin = 11;

/* Colors */
#define COLOR_AMOUNT 7
int colorPointer=0;
/* Loop */
#define LOOP_THREASHOLD 60 
unsigned int loopCounter=8; //toAvoid, that the colors are changed to fast.

/*this is where the row of 15 leds stars on the whole strip*/
int ledsNumber[ROWS] = {
	192,
	168,
	144,
	120,
	96,
	72,
	48,
	24,
	0
};

/*some Rainbow Colors */
//CRGB colors[7] = {
//0x9400D3,
//0x4B0082,       
//0x0000FF,       
//0x00FF00,       
//0xFFFF00,       
//0xFF7F00,       
//0xFF0000
//};
CRGB colors[COLOR_AMOUNT]{
	0xFF8000,
	0x80FF00,       
	0x8000FF,       
	0xFF0000,       
	0x00FF80,       
	0x0080FF,       
	0x4040FF
};

void setup() {
	/*
	 * itit the LEDs
	 */
	FastLED.addLeds<WS2812B, LEDSPIN>(leds, NUM_LEDS); 
	FastLED.setBrightness(10);

	/*
	 * init the Arduino Pins
	 */
	pinMode(analogPin, INPUT);
	pinMode(analogThreasholdPin, INPUT);
	pinMode(strobePin, OUTPUT);
	pinMode(resetPin, OUTPUT);
	pinMode(LEDSPIN, OUTPUT);

	/*
	 * init the MSGEQ7
	 */
	digitalWrite(resetPin, LOW);
	digitalWrite(strobePin, HIGH);

	/*
	 * welcome MSG
	 */
	for (int i = 0; i < NUM_LEDS; i++){
		leds[i]=CRGB::Blue;
		FastLED.show();
		delay(5);
		leds[i]=CRGB::Black;
	}
	delay(1000);
	
	/*
	 * DieBass logo
	 */
	int logo[NUM_LEDS] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,0,0,0,1,0,0,1,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,
		0,1,1,1,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,
		0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,
		0,1,0,0,0,1,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,
		0,1,1,1,1,0,0,0,1,1,1,1,0,1,1,1,0,0,1,1,1,0,0,0
	};
	for (int i = 0; i < NUM_LEDS; i++){
		if (logo[i])
			leds[i]=CRGB::Blue;
		else
			leds[i]=CRGB::Black;
	}
	FastLED.show();
	delay(2000);
	for (int i = 0; i < NUM_LEDS; i++){
		if (logo[i])
			leds[i]=CRGB::Black;
		else
			leds[i]=CRGB::Blue;
	}
	FastLED.show();
	delay(1000);
}

void loop() {
	/*
	 * will contain mapped values
	 */
	int values[COLS]; //30

	/*
	 * Threashold value
	 */
	int t = analogRead(analogThreasholdPin);
	t = t ? t : THREASHOLD_DEFAULT; //default

	/*
	 * init the array
	 */
	for(int i = 0; i < COLS; i++)
		values[i] = 0;

	/*
	 * reset the MSGEQ7
	 */
	digitalWrite(resetPin, HIGH);
	digitalWrite(resetPin, LOW);

	/*
	 * read the values for all 7 bands
	 * and store the mapped value into the array
	 */
	for(int i=2; i < COLS; i+=4){
		digitalWrite(strobePin, LOW);
		delay(1);
		int val = analogRead(analogPin);
		//keep in range
		val = val < 10 ? 0 :  //noise filter
			val > t ? t : // threashold filter (max) 
			val;
		val = val * ROWS / t;
		values[i-1] = values[i] = values[i+1] = val; 
		digitalWrite(strobePin, HIGH);
	}

	/*
	 * interpolate
	 */

	//first	
	values[0] = values[1] / 2;
	//intermediate
	for(int i=4; i<COLS; i += 4){
		values[i] = (values[i-1] + values[i+1]) / 2 ;
	}
	//last-two
	values[COLS-2] = values[COLS-2] % 2; //ceil
	values[COLS-1] = values[COLS-2] / 2; //floor

	/*
	 * light the LEDs
	 */
	for(int freq=0; freq < COLS; freq++){
		if(values[freq]>0){
			for(int depth = 0 ; depth < values[freq]-1; depth++)
				leds[ledsNumber[depth] + freq] = colors[colorPointer];

			leds[ledsNumber[values[freq]-1]+freq] = colors[colorPointer == 0 ? COLOR_AMOUNT : colorPointer - 1 ];//0x00FF00;
		}
		if(values[freq] < ROWS)
			for(int depth = values[freq]; depth < ROWS; depth++)
				leds[ledsNumber[depth] + freq] = CRGB::Black;
	}
	FastLED.show();

	
	/*
	 * move colorpointer, if necessary
	 */
	for(int i=0; i<4; i++)
		if(values[i++] > ROWS/5 && loopCounter < LOOP_THREASHOLD){
			if(colorPointer == COLOR_AMOUNT)
				colorPointer = 0;
			else
				colorPointer++;
			loopCounter = 2 + loopCounter * 2;
			break;
		}
	if(loopCounter > 0)
		loopCounter--;
}
