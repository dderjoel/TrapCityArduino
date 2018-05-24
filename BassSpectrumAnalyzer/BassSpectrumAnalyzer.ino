#include <FastLED.h>
#define NUM_LEDS 216
int analogPin = 0;
int strobePin = 2;
int resetPin = 5;
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
  int ledsNumber[9]= {
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

	for(int i = 0; i<9;i++){
		delay(30);
		leds[ledsNumber[i]]=CRGB::White;
		FastLED.show();
	}

	delay(10000);
}

void loop() {
	int  values23[23];
	int a=0;	
	int t = 280;
	while(a<23)
		values23[a++]=0;

	digitalWrite(resetPin, HIGH);
	digitalWrite(resetPin, LOW);
	for(int i=0;i<23;i+=2){
		digitalWrite(strobePin, LOW);
		delay(1);
		int val = analogRead(analogPin);
		val = val < 10 ? 0 : val > t ? t : val;
		val = val*9/t;
		values23[i] =val; 
		digitalWrite(strobePin, HIGH);
		//Serial.print(values23[i],DEC);
		//Serial.print("\t");
	}
	//Serial.println("next it.");
	
	/*interpolate*/
	for(int i=1;i<21; i+=2){
	//	Serial.println(values23[i],DEC);
		values23[i] = (values23[i-1] + values23[i+1])/ 2 ;
	//	Serial.println(values23[i],DEC);
 	}	
	for(int freq=0; freq < 23; freq++){
		if(values23[freq]>0){
			for(int depth = 0 ; depth < values23[freq]-1; depth++)
				leds[ledsNumber[depth] + freq] = colors[colorPointer];

			leds[ledsNumber[values23[freq]-1]+freq] = colors[colorPointer == 0 ? 5 : colorPointer - 1 ];//0x00FF00;
		}	
		if(values23[freq]<9)
			for(int depth = values23[freq] ; depth < 9; depth++)
				leds[ledsNumber[depth] + freq] = CRGB::Black;
	}
	FastLED.show();

	int b=0;
	while(b<4)
		if(values23[b++]>4 && loopCounter<60){	
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
