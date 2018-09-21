#include <FastLED.h>
#include <Wire.h>

//#define DEBUG

/* definition of LED grid */
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

/* Array to story LED CRGB values */
CRGB leds[num_leds];

/* FastLED library config */
const int leds_pin = 10;
const int led_brightness = 10;
/* Pin for potentiometer to adjust brightness */
const int brightnessThreasholdPin = 1;

/* Colors */
const int color_amount = 7;
int color_pointer = 0;

/* Color Loop */
const int color_loop_threashold=60; 
unsigned int loop_counter = 8; //toAvoid, that the colors are changed to fast.
const int bass_threashold = rows-2;

/* definition of all colors used in the loop */
CRGB colors[color_amount]{
	0xFF8000,
	0x80FF00,       
	0x8000FF,       
	0xFF0000,       
	0x00FF80,       
	0x0080FF,       
	0x4040FF
};

/* data type for state coming over i2c */
union band_states {
  uint8_t bands[cols];
  byte data[cols];
} led_states;

/* function initialises display and prints welcome message */
void initDisplay() {
  FastLED.addLeds<WS2812B, leds_pin, GRB>(leds, num_leds); 
  FastLED.setBrightness(led_brightness);
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
  adjustBrightness();
  /*
   * Bass logo
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
  /* print logo inverted again for LED sanity check */
  for (int i = 0; i < num_leds; i++){
    if (logo[i])
      leds[i]=CRGB::Black;
    else
      leds[i]=CRGB::Blue;
  }
  FastLED.show();
  delay(1000);
  for (int i = 0; i < num_leds; i++){
    if (logo[i])
      leds[i]=CRGB::Blue;
    else
      leds[i]=CRGB::Black;
  }
  FastLED.show();
  delay(2000);
}

/* function to request current state of LEDs over i2c */
void requestLedState() {
  Wire.requestFrom(0, cols);
  for (int i=0; i<cols; i++) {
    led_states.data[i] = Wire.read();
    #ifdef DEBUG
    Serial.print(" ");
    Serial.print(led_states.data[i]);
    Serial.print(" ");
    #endif
  }
  #ifdef DEBUG
  Serial.println("");
  #endif
}

void waitForMusic() {
  while(1) {
    /* request new state */
    requestLedState();
    /* check all bands */
    for(int band=0; band<cols; band++) {
      if(led_states.bands[band] > 0)
        return;
    }
    delay(100);
  }
}

void adjustBrightness() {
  /* map 0 - 1023 -> 0 - 255 */
  int val = analogRead(brightnessThreasholdPin) / 4;
  /* if pulldown resistor sets pin to ground just go with default brightness */
  if (val < 1) return;
  if (val > 255) val = 255;
  FastLED.setBrightness(val);
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(brightnessThreasholdPin, INPUT);
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
  waitForMusic();
}

void loop() {

  /* request new state */
  requestLedState();

  adjustBrightness();

	/* light the LEDs */
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

	/* move colorpointer, if the first eight bands exceed threashold */
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
      /* increase loop_counter by 2 because of decrease below */
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
