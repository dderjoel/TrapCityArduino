#include <FastLED.h>
#include <Wire.h>

//#define DEBUG

const int cols = 24;
const int rows = 9;
const int num_leds = rows*cols;

/*this is where the row of 15 leds stars on the whole strip*/
int ledsNumber[rows] = {
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

CRGB leds[num_leds];

const int leds_pin = 10;

/* Colors */
const int color_amount = 7;
int color_pointer = 0;

/* Loop */
const int color_loop_threashold=30; 
unsigned int loop_counter = 8; //toAvoid, that the colors are changed to fast.
const int bass_threashold = rows-2;

CRGB colors[color_amount]{
	0xFF8000,
	0x80FF00,       
	0x8000FF,       
	0xFF0000,       
	0x00FF80,       
	0x0080FF,       
	0x4040FF
};

union band_states {
  uint8_t bands[cols];
  byte data[cols];
} led_states;

void initDisplay() {
  FastLED.addLeds<WS2812B, leds_pin, GRB>(leds, num_leds); 
  FastLED.setBrightness(10);
   /*
   * Led check
   */
  for (int i = 0; i < num_leds; i++){
    leds[i]=CRGB::Blue;
    FastLED.show();
    delay(5);
    leds[i]=CRGB::Black;
  }
  delay(500);
  
  /*
   * DieBass logo
   */
  int logo[num_leds] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,0,0,1,0,0,1,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,
    0,1,1,1,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,
    0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,
    0,1,0,0,0,1,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,
    0,1,1,1,1,0,0,0,1,1,1,1,0,1,1,1,0,0,1,1,1,0,0,0
  };
  for (int i = 0; i < num_leds; i++){
    if (logo[i])
      leds[i]=CRGB::Blue;
    else
      leds[i]=CRGB::Black;
  }
  FastLED.show();
  delay(2000);
  for (int i = 0; i < num_leds; i++){
    if (logo[i])
      leds[i]=CRGB::Black;
    else
      leds[i]=CRGB::Blue;
  }
  FastLED.show();
  delay(1000);
}

void requestLedState() {
  Wire.requestFrom(0, cols);
  for (int i=0; i<cols; i++) {
    led_states.data[i] = Wire.read();
    #ifdef DEBUG
    //Serial.print(" ");
    //Serial.print(led_states.data[i]);
    //Serial.print(" ");
    #endif
  }
  #ifdef DEBUG
  //Serial.println("");
  #endif
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  #ifdef DEBUG
  delay(1000);
  Serial.begin(9600);
  delay(1000);
  Serial.println("START");
  #endif
  initDisplay();
	Wire.begin();
  delay(100);
  requestLedState();
  //Indicate Setup done
  digitalWrite(13, HIGH);
}

void loop() {

  /*
   * request new state
   */
  requestLedState();

	/*
	 * light the LEDs
	 */
	for(int band=0; band<cols; band++){
    // light up the bars
		if((led_states.bands[band]>0) && (led_states.bands[band]<=rows)) {
			for(int depth = 0 ; depth < led_states.bands[band]-1; depth++)
				leds[ledsNumber[depth] + band] = colors[color_pointer];

			leds[ledsNumber[led_states.bands[band]-1]+band] = colors[color_pointer == 0 ? color_amount : color_pointer - 1 ];
		}
    // delete everything above current bar
		if(led_states.bands[band] < rows)
			for(int depth = led_states.bands[band]; depth < rows; depth++)
				leds[ledsNumber[depth] + band] = CRGB::Black;
	}
	FastLED.show();

	/*
	 * move colorpointer, if the first eight bands exceed threashold
	 */
	for(int i=0; i<8; i++)
		if(led_states.bands[i] > bass_threashold) {
		  if(loop_counter > color_loop_threashold){
			  if(color_pointer >= color_amount)
				  color_pointer = 0;
			  else
				  color_pointer++;
			  loop_counter = 1;
			  break;
		  }
      //
      else loop_counter+=2;
		}

  if(loop_counter>0)
    loop_counter--;

  #ifdef DEBUG
  Serial.print("loop_counter: ");
  Serial.println(loop_counter);
  #endif

  delay(5);
}
