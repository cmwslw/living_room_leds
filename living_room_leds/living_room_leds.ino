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

int currcolor = 0;

#define BTNTHRES 110
#define LEFTCORNER 91
#define CENTERPIX 195
#define NMODES 6
#define NHIST 30
// Set to -1 to turn replay off
int replay_ind = -1;
int hist[NHIST];
int mode = 5;
int lastxdir = 0;
int lastydir = 0;
int record_ind;
int record_time;
int frame = 0;


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  clearhist();
  
  // Enable demo replay
  /*
  hist[0] = 20;
  hist[1] = 20;
  hist[2] = 10;
  hist[3] = 10;
  hist[4] = 10;
  hist[5] = 10;
  hist[6] = -1; // Not actually needed
  replay_ind = 0;
  */
}

void clearhist() {
  for (int i = 0; i < NHIST; i++) {
    hist[i] = -1;
  }
  record_ind = -1;
  record_time = 0;
}

void loop() {
  int x = (analogRead(A0)/4) - 129;
  int y = (analogRead(A1)/4) - 129;
  int ydir = 0;
  int xdir = 0;
  if (y > BTNTHRES)
    ydir = 1;
  else if (y < -BTNTHRES)
    ydir = -1;
  if (x > BTNTHRES)
    xdir = -1;
  else if (x < -BTNTHRES)
    xdir = 1;
    
  if (xdir == 1 && lastxdir == 0) {
    mode += 1;
    clearpixels();
    if (mode > NMODES-1)
      mode = 0;
  } else if (xdir == -1 && lastxdir == 0) {
    mode -= 1;
    clearpixels();
    if (mode < 0)
      mode = NMODES-1;
  }
    
  if (mode == 0) {
    // Always run for expanding modes
    for (int i = 0; i < 4; i += 1) {
      expand();
    }
    currcolor += 1;
    if (currcolor == 256) {
      currcolor = 0;
    }
    
    uint32_t nextcolor = strip.Color(0, 0, 0);
      
    if (ydir == 1 && lastydir == 0) {
      if (replay_ind != -1) {
        replay_ind = -1;
        clearhist();
      } else if (hist[0] != -1) {
        // Start playback if it is in memory
        if (record_ind < NHIST - 1)
          hist[record_ind] = record_time+1;
        record_ind = -1;
        replay_ind = 0;
      }
    } else if (ydir == -1 && lastydir == 0) {
      if (replay_ind != -1) {
        replay_ind = -1;
        clearhist();
      }
      if (record_ind == -1) {
        record_ind = 0;
        record_time = 0;
      } else {
        if (record_ind < NHIST - 1)
          hist[record_ind] = record_time;
        record_ind += 1;
        record_time = 0;
      }
    } else if (ydir == 0 && lastydir == -1) {
      // Must not be playing to record
      if (replay_ind == -1) {
        if (record_ind < NHIST - 1)
          hist[record_ind] = record_time;
        record_ind += 1;
        record_time = 0;
      }
    }
    record_time += 1;
    
    if (replay_ind == -1) {
      //int intensity = abs((analogRead(A1) / 4) - 129);
      int intensity = 0;
      if (ydir != 0) {
        intensity = 128;
      }
      nextcolor = Wheel(currcolor, intensity);
    } else {
      bool on = false;
      int replay_ind_copy = replay_ind;
      int i = 0;
      while (hist[i] != -1 && replay_ind_copy >= 0) {
        if (on) {
          on = false;
        } else {
          on = true;
        }
        replay_ind_copy -= hist[i];
        i += 1;
      }
      if (hist[i] == -1 && replay_ind_copy >= 0)
        replay_ind = 0;
      else
        replay_ind += 1;
      if (on)
        nextcolor = Wheel(currcolor, 128);
    }
     
    strip.setPixelColor(CENTERPIX, nextcolor);
  } else if (mode == 1) { // sparkles
    for (int i = 0; i < PIXEL_COUNT; i += 1) {
      if (random((-y)/4+32+2) == 0)
        strip.setPixelColor(i, 0xAAAAAA);
      else
        strip.setPixelColor(i, 0);
    }
  } else if (mode == 2) { // strobe
    for (int i = LEFTCORNER; i < PIXEL_COUNT; i += 1) {
      if (frame % 4+y/64 >= 2)
        strip.setPixelColor(i, 0x666666);
      else
        strip.setPixelColor(i, 0);
    }
  } else if (mode == 3) {
    static long n = 0;
    for (int i = 0; i < sizeof(long)*8; i++) {
      strip.setPixelColor(i, (n >> i) & 1 ? 0xff0000 : 0);
    }
    n++;
  } else if (mode == 4) {
    for (int i = LEFTCORNER; i < PIXEL_COUNT; i += 1) {
        strip.setPixelColor(i, Wheel(y+128, 128));
    }
  } else if (mode == 5) {
    const int rule = 30;
    const uint32_t automataColor = 0x000060;
    if (frame == 0) {
      // Initial condition
      for (int i = 0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, 0);
      }
      strip.setPixelColor(PIXEL_COUNT-1, automataColor);
    }
    if (frame % 4 == 0) {
      int last = 0;
      int curr = 0;
      int next = 0;
      for (int i = 0; i < PIXEL_COUNT; i++) {
        last = !!strip.getPixelColor(i);
        curr = (i<PIXEL_COUNT) ? !!strip.getPixelColor(i+1) : 0;
        next = (i+1<PIXEL_COUNT) ? !!strip.getPixelColor(i+2) : 0;
        
        int id = next | (curr<<1) | (last<<2);
        int c = !!(rule & (1<<id));
        
        /*if(frame == 1*20 && i < 3) {
          Serial.println("Inner Loop");
          Serial.println(i);
          Serial.println(last);
          Serial.println(curr);
          Serial.println(next);
          Serial.println(id);
          Serial.println(c);
          Serial.print(c);
          Serial.print(" ");
        }
        Serial.println("");*/
        
        strip.setPixelColor(i, c ? automataColor : 0);
      }
    }
  }
  
  strip.show();
  lastydir = ydir;
  lastxdir = xdir;
  frame += 1;
}

void clearpixels() {
  for (int i = 0; i < PIXEL_COUNT; i += 1) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}

void expand() {
  for (int i = LEFTCORNER; i <= CENTERPIX; i += 1) {
    strip.setPixelColor(i-1, strip.getPixelColor(i));
  }
  for (int i = PIXEL_COUNT-1; i >= CENTERPIX; i -= 1) {
    strip.setPixelColor(i+1, strip.getPixelColor(i));
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos, int intensity) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color((255 - WheelPos * 3)*intensity/128, 0, (WheelPos * 3)*intensity/128);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, (WheelPos * 3)*intensity/128, (255 - WheelPos * 3)*intensity/128);
  } else {
   WheelPos -= 170;
   return strip.Color((WheelPos * 3)*intensity/128, (255 - WheelPos * 3)*intensity/128, 0);
  }
}


