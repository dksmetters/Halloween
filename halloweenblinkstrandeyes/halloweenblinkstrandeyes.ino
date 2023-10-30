#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN D3

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
const int numPixels = 50;  
const int numEyes = numPixels/2;
const int maxEyes = 3; // maximum number of concurrently active blinkers

// dead-time between lighting of a range of pixels
const int deadTimeMin = 20;
const int deadTimeMax = 150;

// interval between blink starts - independent of position
const int intervalMin = 10;
const int intervalMax = 300;

const int maxBlink = 80;
const int maxOpen = 100;

const int stepInterval = 10;
long lastStep = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

/*****************************************************************************
Blinker Class

Implements a state machine which generates a blink of random duration and color.
The blink uses two adjacent pixels and ramps the intensity up, then down, with 
a random repeat now and again.
*****************************************************************************/

class blinker
{
  public:
  
  boolean m_active;  // blinker is in use.
  int m_deadTime;  // don't re-use this pair immediately
  
  int m_pos;  // position of the 'left' eye.  the 'right' eye is m_pos + 1
  
  int m_red;  // RGB components of the color
  int m_green;
  int m_blue;
  uint32_t m_color;

  int m_duration; 
  int m_close_duration;
  int m_repeats;  // not used
  
  public:
  // Constructor - start as inactive
  blinker()
  {
    m_active = false;
  }

  
  // Create a 24 bit color value from R,G,B
  uint32_t Color(byte r, byte g, byte b)
  {
    uint32_t c;
    c = r;
    c <<= 8;
    c |= g;
    c <<= 8;
    c |= b;
    return c;
  }
  
  // Initiate a blink at the specified pixel position
  // All other blink parameters are randomly generated
  void StartBlink(int pos)
  {
    m_pos = pos;
    
    // Pick a random color - skew toward red/orange/yellow part of the spectrum for extra creepyness
    m_red = random(230, 255);
    m_blue = 0;
    m_green = random(0,20);
    m_color = Color(m_green, m_red, m_blue);

    m_repeats += random(1, 3);
    
    // set blink speed and deadtime between blinks
    m_duration = random(20, 1000);
    m_deadTime = random(deadTimeMin, deadTimeMax);
    // close blink time
    m_close_duration = 0;

    // Mark as active and start at intensity zero
    m_active = true;
  }
  
  // Step the state machine:
  void step()
  {
    if (!m_active)
    { 
      // count down the dead-time when the blink is done
      if (m_deadTime > 0)
      {
        m_deadTime--;
      }
      return;
    }
    strip.setPixelColor(m_pos, m_color);
    strip.setPixelColor(m_pos +1, m_color);

    m_duration--;
    if (m_duration <= 0 || m_close_duration > 0)
    {
        // make sure pixels all are off
      strip.setPixelColor(m_pos, Color(0,0,0));
      strip.setPixelColor(m_pos+1, Color(0,0,0));

      if (m_close_duration > 0) {
        m_close_duration--;
      } else if (--m_repeats <= 0)      // Are we done?
      {
         m_active = false;
      }
      else // no - start to ramp up again
      {
          m_close_duration = random(5, maxBlink);
          m_duration = random(1, maxOpen) + m_close_duration;
      }
      return;
    }
    
     
  }
};

// An array of blinkers - this is the maximum number of concurrently active blinks
blinker blinkers[maxEyes];

// A delay between starting new blinks
int countdown;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  Serial.begin(9600);
  randomSeed(analogRead(0));

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop()
{
  if (millis() - lastStep > stepInterval)
  {
    lastStep = millis();
    --countdown;
    for(int i = 0; i < maxEyes; i++)
    {
      // Only start a blink if the countdown is expired and there is an available blinker
      if ((countdown <= 0) && (blinkers[i].m_active == false))
      {
        int newPos = random(0, numPixels/2) * 2;
            
        for(int j = 0; j < maxEyes; j++)
        {
          // avoid active or recently active pixels
          if ((blinkers[j].m_deadTime > 0) && (abs(newPos - blinkers[j].m_pos) < 4))
          {
            Serial.print("-");
            Serial.print(newPos);
            newPos = -1;  // collision - do not start
            break;
          }
        }
  
        if (newPos >= 0)  // if we have a valid pixel to start with...
        {
         Serial.print(i);
         Serial.print(" Activate - ");
         Serial.println(newPos);
         blinkers[i].StartBlink(newPos);  
         countdown = random(intervalMin, intervalMax);  // random delay to next start
        }
      }
      // step all the state machines
       blinkers[i].step();
    }
    // update the strip
    strip.show();
  }
}
