#include <Arduino.h>
#include "Servo.h"
#include "SPI.h"

// #include <stdint.h>  // check if needed, included for pointer refrence to currentMillis

#include "RFID.h"
#include "Led.h"
#include "Key.h"
#include "Keypad.h"

Led ledRed(32);
Led ledYellow(33);
Led ledGreen(34);
Led ledStatus(LED_BUILTIN);

Led ledTest(12);

// GLOBAL VARIABLEs
unsigned long currentMillis;
bool directionUp; // if true, then get brighter, if false get dimmer

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // LEDs
  ledStatus.init();
  ledRed.init();
  ledYellow.init();
  ledGreen.init();

  ledTest.init();



  // KEYPAD
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();
  Serial.println("current runtime: " + (String)currentMillis);
  unsigned long* pCurrentMillis = &currentMillis;

  ledStatus.toggle();

  ledTest.fadeIn(pCurrentMillis,50000);
  ledGreen.fadeIn(pCurrentMillis,5000);
  ledRed.fadeIn(pCurrentMillis,10000);
  ledYellow.fadeIn(pCurrentMillis,1000);
  delay(5);
}
