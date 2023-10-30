// Combination sketch that drives two LED backpacks (off SCL and SDA)
// and sound off the SparkFun MP3Player shield on an Uno and takes a motion
// sensor on A0. 
// Combines:
// 'roboface' example sketch for Adafruit I2C 8x8 LED backpack
// DoorOpenPlayTrack for sound and motion (which is geared to the
// SparkFun MP3 shield).
//
//  www.adafruit.com/products/870   www.adafruit.com/products/1049
//  www.adafruit.com/products/871   www.adafruit.com/products/1050
//  www.adafruit.com/products/872   www.adafruit.com/products/1051
//  www.adafruit.com/products/959   www.adafruit.com/products/1052
//
// Requires Adafruit_LEDBackpack and Adafruit_GFX libraries.
// For a simpler introduction, see the 'matrix8x8' example.
//
// This sketch demonstrates a couple of useful techniques:
// 1) Addressing multiple matrices (using the 'A0' and 'A1' solder
//    pads on the back to select unique I2C addresses for each).
// 2) Displaying the same data on multiple matrices by sharing the
//    same I2C address.
//
// This example uses 5 matrices at 4 addresses (two share an address)
// to animate a face:
//
//     0     0
//
//      1 2 3
//
// The 'eyes' both display the same image (always looking the same
// direction -- can't go cross-eyed) and thus share the same address
// (0x70).  The three matrices forming the mouth have unique addresses
// (0x71, 0x72 and 0x73).
//
// The face animation as written is here semi-random; this neither
// generates nor responds to actual sound, it's simply a visual effect
// Consider this a step1``````````````````````````````````g off point for your own project.  Maybe you
// could 'puppet' the face using joysticks, or synchronize the lips to
// audio from a Wave Shield (see wavface example).  Currently there are
// only six images for the mouth.  This is often sufficient for simple
// animation, as explained here:
// http://www.idleworm.com/how/anm/03t/talk1.shtml
//
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
//
// Written by P. Burgess for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
// include SPI, MP3 and SD libraries
// For SparkFun MP3 player shield
#include <SPI.h>
//Add the SdFat Libraries
#include <SdFat.h>
//and the MP3 Shield Library
#include <SFEMP3Shield.h>

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

// Because the two eye matrices share the same address, only four
// matrix objects are needed for the five displays:
#define MATRIX_EYES         0

#define MOTION_THRESHOLD 500
#define PLAYING_THRESHOLD 500


Adafruit_8x8matrix matrix[1] = { // Array of Adafruit_8x8matrix objects
  Adafruit_8x8matrix() };

SdFat sd;
//create and name the library object
// Plays tracks in order. Tracks must be named
// track000.mp3, track001.mp3, etc. Set maxTrack to determine how many
// to look for.
SFEMP3Shield MP3player;
void printDetail(uint8_t type, int value);
void drawEyes(bool sensingMotion);

// Rather than assigning matrix addresses sequentially in a loop, each
// has a spot in this array.  This makes it easier if you inadvertently
// install one or more matrices in the wrong physical position --
// re-order the addresses in this table and you can still refer to
// matrices by index above, no other code or wiring needs to change.
static const uint8_t matrixAddr[] = { 0x70 };
static const uint8_t matrixCount = 1;

static const uint8_t PROGMEM // Bitmaps are stored in program memory
  blinkImg[][8] = {    // Eye animation frames
  { B00111100,         // Fully open eye
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B00000000,
    B00111100,
    B11111111,
    B11111111,
    B11111111,
    B00111100,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00111100,
    B11111111,
    B01111110,
    B00011000,
    B00000000 },
  { B00000000,         // Fully closed eye
    B00000000,
    B00000000,
    B00000000,
    B10000001,
    B01111110,
    B00000000,
    B00000000 } },
  rotBlinkImg[][8] = {    // Eye animation frames
  { B00111100,            // Fully open eye
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00111100,
    B01111110,
    B01111111,
    B01111111,
    B01111111,
    B01111111,
    B01111110,
    B00111100 },
  { B00011100,
    B00011100,
    B00111110,
    B00111110,
    B00111110,
    B00111110,
    B00011100,
    B00011100 },
  { B00001000,
    B00001100,
    B00011100,
    B00011110,
    B00011110,
    B00011100,
    B00001100,
    B00001000 },
  { B00001000,
    B00000100,
    B00000100,
    B00000100,
    B00000100,
    B00000100,
    B00000100,
    B00001000 }
    };
#define EYEX_CENTER 3
#define EYEY_CENTER 3

uint8_t
  blinkIndex[] = { 1, 2, 3, 4, 3, 2, 1 }, // Blink bitmap sequence
  blinkCountdown = 100, // Countdown to next blink (in frames)
  gazeCountdown  =  75, // Countdown to next eye movement
  gazeFrames     =  50; // Duration of eye movement (smaller = faster)
int8_t
  eyeX = EYEX_CENTER, eyeY = EYEY_CENTER,   // Current eye position
  newX = 3, newY = 3,   // Next eye position
  dX   = 0, dY   = 0;   // Distance from prior to new position

int index = 0;
bool sensingMotion = false;
bool playing = false;
int maxTrack = 7;
byte currentTrack = 0;

byte temp;
byte result;
char title[30];
int ledPin = 13;      // select the pin for the LED

void setup() {
  Serial.begin(115200);
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  // Seed random number generator from an unused analog input:
  randomSeed(analogRead(A2));

  Serial.println();
  Serial.println(F("HalloweenSkull Audio and Eyes"));
  Serial.println(F("Initializing eyes..."));
  // Initialize each matrix object:
  for(uint8_t i=0; i<matrixCount; i++) {
    matrix[i].begin(matrixAddr[i]);
    // If using 'small' (1.2") displays vs. 'mini' (0.8"), enable this:
    // matrix[i].setRotation(3);
  }

  Serial.println(F("Initializing SparkFun player ... (May take 3~5 seconds)"));
  //boot up the MP3 Player Shield
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  // depending upon your SdCard environment, SPI_HAVE_SPEED may work better.
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
  result = MP3player.begin();
  //check result, see readme for error codes.
  if (result != 0) {
    Serial.print("Error code: ");
    Serial.print(result);
    Serial.println(" when trying to start MP3 player");
  } else {
    Serial.println("Ready to start!");
  }
  MP3player.setVolume(10,10);
}


void loop() {
  sensingMotion = senseMotion();
  playing = isPlaying();
  drawEyes(sensingMotion);
  // Refresh all of the matrices in one quick pass
  for(uint8_t i=0; i<matrixCount; i++) matrix[i].writeDisplay();
  if (sensingMotion && !playing) {
    playing = true;
    playNextTrack();
  }

  delay(20); // ~50 FPS
}

bool senseMotion() {
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.print("Motion sensor value: ");
  Serial.println(sensorValue);
 return sensorValue > MOTION_THRESHOLD;
}

bool isPlaying() {
  return MP3player.isPlaying();
}

int playNextTrack() {
  currentTrack++;
  if (currentTrack > maxTrack) {
    currentTrack = 1;
  }
  // play the track
  Serial.print("Playing track: ");
  Serial.println(currentTrack);
  result = MP3player.playTrack(currentTrack);
  if (result != 0) {
    Serial.print("Error code: ");
    Serial.print(result);
    Serial.println(" when trying to play track");
  } else {
    Serial.println("Playing!");
  }
  // can do MP3player.stopTrack();
}


void drawEyes(bool sensingMotion) {
   // Draw eyeball in current state of blinkyness (no pupil).  Note that
  // only one eye needs to be drawn.  Because the two eye matrices share
  // the same address, the same data will be received by both.
  matrix[MATRIX_EYES].clear();
  // When counting down to the next blink, show the eye in the fully-
  // open state.  On the last few counts (during the blink), look up
  // the corresponding bitmap index.
  matrix[MATRIX_EYES].drawBitmap(0, 0,
    blinkImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
    ], 8, 8, LED_ON);
  // Decrement blink counter.  At end, set random time for next blink.
  if (--blinkCountdown == 0) blinkCountdown = random(5, 180);

  // Add a pupil (2x2 black square) atop the blinky eyeball bitmap.
  // Periodically, the pupil moves to a new position...
  // If we're sensing motion, though, put the eyes at the center.
  if (!sensingMotion && (--gazeCountdown <= gazeFrames)) {
    // Eyes are in motion - draw pupil at interim position
    matrix[MATRIX_EYES].fillRect(
      newX - (dX * gazeCountdown / gazeFrames),
      newY - (dY * gazeCountdown / gazeFrames),
      2, 2, LED_OFF);
    if (gazeCountdown == 0) {    // Last frame?
      eyeX = newX; eyeY = newY; // Yes.  What's new is old, then...
      do { // Pick random positions until one is within the eye circle
        newX = random(7); newY = random(7);
        dX   = newX - 3;  dY   = newY - 3;
      } while((dX * dX + dY * dY) >= 10);      // Thank you Pythagoras
      dX            = newX - eyeX;             // Horizontal distance to move
      dY            = newY - eyeY;             // Vertical distance to move
      gazeFrames    = random(3, 15);           // Duration of eye movement
      gazeCountdown = random(gazeFrames, 120); // Count to end of next movement
    }
  } else {
    // Not in motion -- draw pupil at current static position
    if (sensingMotion) {
      eyeX = EYEX_CENTER;
      eyeY = EYEY_CENTER;
   }
    matrix[MATRIX_EYES].fillRect(eyeX, eyeY, 2, 2, LED_OFF);
  }
}
