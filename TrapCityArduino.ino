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
unsigned const int silence_threashold = 500;
unsigned int silence_counter = 0;
bool silence = false;

/* Array to story LED CRGB values */
CRGB leds[num_leds];

/* FastLED library config */
const int leds_pin = 5;
int led_brightness = 50;

/* Pin for potentiometer to adjust brightness */
const int brightnessThreasholdPin = 1;

/* Colors */
const int color_amount = 7;
int color_pointer = 0;

/* Color Loop */
const int color_loop_threashold=30; 
unsigned int loop_counter = 8; //toAvoid, that the colors are changed to fast.
const int bass_threashold = rows-2;

/* definition of all colors used in the loop */
CRGB colors[color_amount]{
	0xFF8000,
	0x80FF00,       
	0x8000FF,       
	0xFF0000,       
	0x00FF80,       
	0xFFFF00,       
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
  adjustBrightness();
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
  
  /* print logo inverted again for LED sanity check */
  for (int i = 0; i < num_leds; i++){
    if (logo[i])
      leds[i]=CRGB::Black;
    else
      leds[i]=CRGB::Blue;
  }
  FastLED.show();
  delay(1000);
  showLogo();
  delay(2000);
}

void showLogo(){
  for (int i = 0; i < num_leds; i++){
    if (logo[i])
      leds[i]=CRGB::Blue;
    else
      leds[i]=CRGB::Black;
  }
  FastLED.show();
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
  bool brightness_increase = false;
  unsigned int brightness_cur = 0;
  float brightness_increase_delay = ((255 / led_brightness) - 1) * 10;
  unsigned int brightness_increase_delay_count = 0;
  unsigned int request_delay_counter = 50;
  #ifdef DEBUG
  bool led_state = true;
  #endif
  
  while(1) {
    if (request_delay_counter > 49) {
      request_delay_counter = 0;
      /* request new state */
      requestLedState();
      /* check all bands */
      for(int band=0; band<cols; band++) {
        if(led_states.bands[band] > 1){
          FastLED.setBrightness(led_brightness);
          return;
        }
      }
    } else request_delay_counter++;

    adjustBrightness();
    if (led_brightness > 10) {
      brightness_increase_delay = ((255 / led_brightness) - 1) * 10;
      if (brightness_increase_delay_count > ((int)brightness_increase_delay)) {
        brightness_increase_delay_count = 0;
        if (brightness_cur < 2)
          brightness_increase = true;
        else if (brightness_cur > (led_brightness - 2))
          brightness_increase = false;
    
        if (brightness_increase)
          brightness_cur += 2;
        else
          brightness_cur -= 2;
    
        FastLED.setBrightness(brightness_cur);
        showLogo();

        #ifdef DEBUG
        if(led_state) digitalWrite(13, HIGH);
        else digitalWrite(13, LOW);
        led_state = !led_state;
        #endif
        
      }
      else
        brightness_increase_delay_count++;
    } else {
      FastLED.setBrightness(led_brightness);
      showLogo();
    }
      
    delay(1);
  }
}

void clearDisplay() {
  for (int i = 0; i < num_leds; i++)
    leds[i]=CRGB::Black;
}

void adjustBrightness() {
  /* map 0 - 1023 -> 0 - 255 */
  int val = analogRead(brightnessThreasholdPin) / 4;
  /* if pulldown resistor sets pin to ground just go with default brightness */
  if (val < 1) return;
  if (val > 255) val = 255;
  led_brightness = val;
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
  bool silence = true;

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
      silence = false;
      silence_counter = 0;
		}
    // delete everything above current bar
		if(led_states.bands[band] < rows)
			for(int depth = led_states.bands[band]; depth < rows; depth++)
				leds[ledsNumber[depth] + band] = CRGB::Black;
	}
  if(silence_counter <= silence_threashold && silence)
    silence_counter++;
  else if(silence_counter > silence_threashold) {
    waitForMusic();
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
