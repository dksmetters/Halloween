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
const int numPixels = 12;  
const int numEyes = numPixels;
const int maxEyes = 10; // maximum number of concurrently active blinkers

// dead-time between lighting of a range of pixels
const int deadTimeMin = 5;
const int deadTimeMax = 20;

// how long each pixel is on
const int liveTimeMin = 10;
const int liveTimeMax = 80;

// ms between starts
const int intervalMin = 1;
const int intervalMax = 20;

const int stepInterval = 2;
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
  int m_liveTime; // how long this blink is
  
  int m_pos;  // position of the 'left' eye.  the 'right' eye is m_pos + 1
  
  int m_color; // choice of red/orange/yellow

  // Here we divide each pixel into fixed colors - red, yellow, orange in rgb
  uint32_t red = Color(255, 0, 0);
  uint32_t yellow = Color(255, 180, 0);
  uint32_t orange = Color(255, 80, 0);
  uint32_t colors[3] = {red, orange, yellow};

  
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
    
    // set blink speed and deadtime between blinks
    m_deadTime = random(deadTimeMin, deadTimeMax);

    // Mark as active and start at intensity zero
    m_active = true;
    m_color = random(0,3);
    m_liveTime = random(liveTimeMin,liveTimeMax);
    // Treating each pixel alone
    strip.setPixelColor(m_pos, colors[m_color]);
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
    
    // We're active, count down how long
    m_liveTime--; 
    if (m_liveTime <= 0) {
      // make sure pixels all are off
      strip.setPixelColor(m_pos, Color(0,0,0));
      m_active = false;
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
  for (int i=0; i< numPixels; i++) {
     strip.setPixelColor(i, 0);
  }
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
        int newPos = random(0, numEyes);
            
        for(int j = 0; j < maxEyes; j++)
        {
          // avoid active or recently active pixels
          if ((blinkers[j].m_deadTime > 0) && (abs(newPos - blinkers[j].m_pos) < 2))
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
