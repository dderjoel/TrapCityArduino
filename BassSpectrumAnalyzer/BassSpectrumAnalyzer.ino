#include <FastLED.h>
#define NUM_LEDS 240
int analogPin = 0;
int strobePin = 2;
int resetPin = 3;
#define LEDSPIN 10

int values[7];
int colorPointer=0;
unsigned int loopCounter=8; //toAvoid, that the colors are changed to fast.
CRGB leds[NUM_LEDS];
/*
//Xx Xx Xx Xx Xx Xx X 
0/01 23 45 67 89 01 2
1/01 23 45 67 89 01 2
2/01 23 45 67 89 01 2
3/01 23 45 67 89 01 2
4/01 23 45 67 89 01 2
5/01 23 45 67 89 01 2
 */

/*this is where the row of 15 leds stars on the whole strip*/
  int ledsNumber[6]= {
  227,
  186,
  140,
  96,
  53,
  3
  };

/*some Rainbow Colors */
//CRGB colors[7] = {
//	0x9400D3,
//	0x4B0082,       
//	0x0000FF,       
//	0x00FF00,       
//	0xFFFF00,       
//	0xFF7F00,       
//	0xFF0000
//};
CRGB colors[7] = {
	0xFF8000,
	0x80FF00,       
	0x8000FF,       
	0xFF0000,       
	0x00FF80,       
	0x0080FF,       
	0x4040FF
};

void setup() {
	FastLED.addLeds<WS2812B, LEDSPIN>(leds, NUM_LEDS); 
//	Serial.begin(9600);
	pinMode(analogPin, INPUT);
	pinMode(strobePin, OUTPUT);
	pinMode(resetPin, OUTPUT);
	pinMode(LEDSPIN, OUTPUT);

	digitalWrite(resetPin, LOW);
	digitalWrite(strobePin, HIGH);
	FastLED.setBrightness(10);
	for(int i = 0; i<NUM_LEDS; i++){
		leds[i]=CRGB::Blue;
		FastLED.show();
		leds[i]=CRGB::Black;
	}

	for(int i = 0; i<6;i++){
		delay(10);
		leds[ledsNumber[i]]=CRGB::White;
		FastLED.show();
	}

	delay(1000);
}

void loop() {
	int  values13[13];
	int a=0;	
	int t = 280;
	while(a<13)
		values13[a++]=0;

	digitalWrite(resetPin, HIGH);
	digitalWrite(resetPin, LOW);
	for(int i=0;i<13;i+=2){
		digitalWrite(strobePin, LOW);
		delay(1);
		int val = analogRead(analogPin);
		val = val < 10 ? 0 : val > t ? t : val;
		val = val*6/t;
		values13[i] =val; 
		digitalWrite(strobePin, HIGH);
		//Serial.print(values13[i],DEC);
		//Serial.print("\t");
	}
	//Serial.println("next it.");
	
	/*interpolate*/
	for(int i=1;i<12; i+=2){
	//	Serial.println(values13[i],DEC);
		values13[i] = (values13[i-1] + values13[i+1])/ 2 ;
	//	Serial.println(values13[i],DEC);
 	}	
	for(int freq=0; freq < 13; freq++){
		if(values13[freq]>0){
			for(int depth = 0 ; depth < values13[freq]-1; depth++)
				leds[ledsNumber[depth] + freq] = colors[colorPointer];

			leds[ledsNumber[values13[freq]-1]+freq] = colors[colorPointer == 0 ? 5 : colorPointer - 1 ];//0x00FF00;
		}	
		if(values13[freq]<6)
			for(int depth = values13[freq] ; depth < 6; depth++)
				leds[ledsNumber[depth] + freq] = CRGB::Black;
	}
	FastLED.show();

	int b=0;
	while(b<4)
		if(values13[b++]>4 && loopCounter<60){	
			if(colorPointer==6)
				colorPointer=0;
			else
				 colorPointer++;
			loopCounter = 2+ loopCounter * 2;
			break;
		}	
	if(loopCounter > 0)
		loopCounter--;
}
