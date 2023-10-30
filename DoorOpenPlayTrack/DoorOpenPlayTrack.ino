#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <FreeStack.h>

//and the MP3 Shield Library
#include <SFEMP3Shield.h>
/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13. 
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead(). 
 
 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground
 
 * Note: because most Arduinos have a built-in LED attached 
 to pin 13 on the board, the LED is optional.
 
 
 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe
 
 This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/AnalogInput
 
 */

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
int thresholdValue = 400;
int loopTime = 100;
int maxTrack = 7;
int currentTrack = 0;

/**
 * \brief Object instancing the SdFat library.
 *
 * principal object for handling all SdCard functions.
 */
SdFat sd;

//create and name the library object
SFEMP3Shield MP3player;

byte temp;
byte result;
char title[30];

void setup() {
  uint8_t result; //result code from some function as to be tested at later time.

  Serial.begin(115200);

  Serial.print(F("F_CPU = "));
  Serial.println(F_CPU);
  Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
  Serial.print(FreeStack(), DEC);  // FreeStack() is provided by SdFat
  Serial.println(F(" Should be a base line of 1028, on ATmega328 when using INTx"));
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  //boot up the MP3 Player Shield
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_FULL_SPEED)) sd.initErrorHalt();
  // depending upon your SdCard environment, SPI_HAVE_SPEED may work better.
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    if( result == 6 ) {
      Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
      Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
    }
  }
  Serial.println("Ready to start!");
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin); 
  Serial.print("Sensor value: ");
  Serial.println(sensorValue);
  if (sensorValue > thresholdValue) {
    if (!MP3player.isPlaying()) {
       // turn the ledPin on
      digitalWrite(ledPin, HIGH);
      // pick a track
      currentTrack++;
      if (currentTrack > maxTrack) {
       currentTrack = 1;
      }
     // play the track
      result = MP3player.playTrack(currentTrack);
      if (result != 0) {
        Serial.print("Error code: ");
        Serial.print(result);
        Serial.println(" when trying to play track");
      } else {
        Serial.println("Playing!");
      }
    }
  } else {
    digitalWrite(ledPin, LOW);
    MP3player.stopTrack();
    Serial.println("Stopping!");
  }
  // stop the program for <sensorValue> milliseconds:
  delay(loopTime);          
}
