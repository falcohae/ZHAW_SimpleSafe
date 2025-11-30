#include <Arduino.h>
#include <Servo.h>

#include "SPI.h"
#include "RFID.h"

#define SDA_DIO 8
#define RESET_DIO 9

RFID RC522(SDA_DIO, RESET_DIO);
Servo servo;

#define SERVO_PIN 10
#define SERVO_VERRIEGELT   60
#define SERVO_ENTRIEGELT SERVO_VERRIEGELT + 120
#define SERVO_DELAY 5
#define SERVO_DISABLE 0 

struct rfidCard {
  unsigned char serNum[5];
};

struct rfidCard allowedCards[5];

int angle = SERVO_VERRIEGELT;

void setup()
{
  Serial.begin(9600);
  /* Enable the SPI interface */
  SPI.begin();
  /* Initialise the RFID reader */
  RC522.init();
  
  servo.attach(SERVO_PIN);

  Serial.println("Leggo!!!");

  allowedCards[3].serNum[0] = 0xB3;
  allowedCards[3].serNum[1] = 0x73;
  allowedCards[3].serNum[2] = 0xF4;
  allowedCards[3].serNum[3] = 0x2C;
  allowedCards[3].serNum[4] = 0x18;
  

}

void lockDoor() {
  Serial.println("Verriegle Tuere...");
#if SERVO_DISABLE == 0
  for(int i = SERVO_ENTRIEGELT; i > SERVO_VERRIEGELT; i--) {
    servo.write(i);
    delay(SERVO_DELAY);
  }
  servo.write(SERVO_VERRIEGELT);
#else
  Serial.println("Verriegle Tuere... Simulation.");
#endif
  Serial.println("Verriegle Tuere... abgeschlossen.");
}

void unlockDoor() {
  Serial.println("Entriegle Tuere...");
#if SERVO_DISABLE == 0
  for(int i = SERVO_VERRIEGELT; i < SERVO_ENTRIEGELT; i++) {
    servo.write(i);
    delay(SERVO_DELAY);
  }
  servo.write(SERVO_ENTRIEGELT);
#else
  Serial.println("Entriegle Tuere... Simulation.");
#endif
  Serial.println("Entriegle Tuere... abgeschlossen.");
}

bool isAllowedCard(unsigned char CardSerial[5])
{

  for (int i = 0; i < 5; i++)
  {
    Serial.print(CardSerial[i], HEX); //to print card detail in Hexa Decimal format
  }
  Serial.println();
  
  bool foundValidCard = false;

  for(int i = 0; i < sizeof(allowedCards)/sizeof(rfidCard); i++)
  {
    Serial.print("Testing against card ");
    Serial.println(i);
    if(!strcmp((char*)allowedCards[i].serNum, (char*)CardSerial)){
      Serial.print(i);
      Serial.println(" matched.");
      foundValidCard = true;
      exit;
    }
  }
  Serial.println();
  return foundValidCard;
}

void waitForValidCard()
{
  Serial.print("waitForValidCard");
  while (true)
  {
    Serial.print(".");
    delay(333);
    if(RC522.isCard()) {
      Serial.println(" detected!");
      RC522.readCardSerial();
      if (isAllowedCard(RC522.serNum)) {
        Serial.println("waitForValidCard... done.");
        return;
      } else {
        Serial.println("waitForValidCard... reset.");
      }
    }
  }
}

void tresorMain()
{
  // door is locked

  // wait for valid card
  waitForValidCard();

  // unlock door
  unlockDoor();
  
  delay(1000);

  // wait for valid card
  waitForValidCard();

  // lock door
  lockDoor();
  
  delay(1000);
}

void loop()
{
  tresorMain();
}
