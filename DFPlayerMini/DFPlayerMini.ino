#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
 
#define PIN_busy D7
#define PIN_RX D5
#define PIN_TX D6
 
SoftwareSerial ss(PIN_RX, PIN_TX);  //RX, TX
DFRobotDFPlayerMini Player;
 
void setup() {
  ss.begin(9600);
  Serial.begin(9600);
  delay(1000);
  if (!Player.begin(ss)) {
    Serial.println("errore mp3");
    for (;;)
      ;
 
    //Valore iniziale del volume
    Player.volume(1);
  }
}
 
void loop() {
  Player.play(11);
  delay(1000);
  while (digitalRead(PIN_busy) == LOW) {
    delay(1);
  }
  avanti();
  delay(1000);
  indietro();
}
 
//Lettura numeri in avanti
void avanti() {
  for (int i = 1; i <= 10; i = i + 1) {
    Player.play(i);
    Serial.print("Valore di i: ");
    Serial.println(i);
    delay(1000);
  }
}
//Lettura numeri all'indietro
void indietro() {
  for (int i = 10; i >= 1; i = i - 1) {
    Player.play(i);
    Serial.print("Valore di i: ");
    Serial.println(i);
    delay(1000);
  }
}

