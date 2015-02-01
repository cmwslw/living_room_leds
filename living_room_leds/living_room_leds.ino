#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN   2    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    4    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 300

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
int updates = 0;
int currcolor = 0;
bool on = true;

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  for (int i = 0; i < 4; i += 1) {
    expand();
  }
  updates += 1;
  currcolor += 1;
  if (updates == 10) {
    if (on) {
      on = false;
    } else {
      on = true;
    }
    updates = 0;
  }
  if (currcolor == 256) {
    currcolor = 0;
  }
  uint32_t nextcolor = strip.Color(0, 0, 0);
  if (on) {
    nextcolor = Wheel(currcolor);
  }
    
  strip.setPixelColor(195, nextcolor);
  strip.show();
}

void expand() {
  for (int i = 1; i <= 195; i += 1) {
    strip.setPixelColor(i-1, strip.getPixelColor(i));
  }
  for (int i = 299; i >= 195; i -= 1) {
    strip.setPixelColor(i+1, strip.getPixelColor(i));
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
