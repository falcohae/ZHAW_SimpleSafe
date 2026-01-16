#include <Arduino.h>
#include "Servo.h"
#include "SPI.h"

#include "RFID.h"
#include "Led.h"
#include "Key.h"
#include "Keypad.h"

// GLOBAL VARIABLEs
unsigned long currentMillis;
bool directionUp; // if true, then get brighter, if false get dimmer
enum StateHandler {CHECK_USER, CHECK_PIN, OPENING, UNLOCKED, CLOSING, LOCKED, THEFT, SHOCK};
enum StateHandler stateMachine;

#define SDA_DIO 8
#define RESET_DIO 9
#define SERVO_PIN 10
#define SERVO_VERRIEGELT   0
#define SERVO_ENTRIEGELT 90
#define SERVO_DELAY 5
#define SERVO_SIMULATE false
#define PINLENGTH 4

struct rfidCard {
  unsigned char serNum[5];
  unsigned char pin[4];
//  bool allowOpen = false;
//  bool allowClose = false;
//  bool allowArm = false;
//  bool allowUnarm = false;
//  bool allowMute = false;
};

struct rfidCard allowedCards[5];
rfidCard *currentCard;

// Objects
Led ledRed(4);
Led ledYellow(3);
Led ledGreen(2);
Led buzzer(7);  // LED object can be used for buzzer to enhance handling
Led ledStatus(LED_BUILTIN);

Led ledTest(12);

RFID RC522(SDA_DIO, RESET_DIO);
Servo servo;

// KEYPAD

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte colPins[ROWS] = {28, 26, 24, 22}; //connect to the column pinouts of the keypad
byte rowPins[COLS] = {36, 34, 32, 30}; //connect to the row pinouts of the keypad

Keypad pinpad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

// ******************** FUNCTION CODE ******************** //

bool isKnownCard(unsigned char CardSerial[5]) {

  for (int i = 0; i < 5; i++) {
    Serial.print(CardSerial[i], HEX); //to print card detail in Hexa Decimal format
  }
  Serial.println();
  
  bool foundValidCard = false;

  for(unsigned int i = 0; i < sizeof(allowedCards)/sizeof(rfidCard); i++) {
    Serial.print("Testing against card ");
    Serial.println(i);
    if(!memcmp((char*)allowedCards[i].serNum, (char*)CardSerial, 5)){
      Serial.print(i);
      Serial.println(" matched.");
      currentCard = &allowedCards[i];
      foundValidCard = true;
      break;
    }
  }
  Serial.println();
  return foundValidCard;
};

bool unlockDoor() {
  Serial.println("Entriegle Tuere...");
  #if (SERVO_SIMULATE)    // only evaluated at compile time
    Serial.println("Entriegle Tuere... Simulation.");
    delay(500);
    return true;
  #else

    for(int i = SERVO_VERRIEGELT; i < SERVO_ENTRIEGELT; i++) {
      servo.write(i);
      delay(SERVO_DELAY);
    };
    servo.write(SERVO_ENTRIEGELT);

    return (servo.read() >= SERVO_ENTRIEGELT);
  #endif
  Serial.println("Entriegle Tuere... abgeschlossen.");
};

bool lockDoor() {
  Serial.println("Verriegle Tuere...");
  #if (SERVO_SIMULATE)
    Serial.println("Verriegle Tuere... Simulation.");
  #else
    for(int i = SERVO_ENTRIEGELT; i > SERVO_VERRIEGELT; i--) {
      servo.write(i);
      delay(SERVO_DELAY);
    };
    servo.write(SERVO_VERRIEGELT);
  #endif
    Serial.println("Verriegle Tuere... abgeschlossen.");

  return (servo.read() >= SERVO_VERRIEGELT);
};


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
      if (isKnownCard(RC522.serNum)) {
        Serial.println("waitForValidCard... done.");
        return;
      } else {
        Serial.println("waitForValidCard... reset.");
      }
    }
  }
}

char readButtonPress() {
    
    char buttonPressed = 0;

    while(!buttonPressed) {
      buttonPressed = pinpad.getKey();
    };
    

    Serial.print("Button ");
    Serial.print(buttonPressed, HEX);
    Serial.print(" was pressed");

    delay(200); // debounce
    return buttonPressed;
}

bool validatePin(unsigned char pinToValidate[]) {
  Serial.println("validatePin");

  Serial.print("Validating against PIN ");

  bool validationSuccessfull = true;

  for(int i = 0; i < PINLENGTH; i++) {
    Serial.print(pinToValidate[i], HEX);
    Serial.print("|");
  }
  Serial.println();

  for(int i = 0; i < PINLENGTH; i++) {
    char buttonPressed = readButtonPress();

    if(buttonPressed != pinToValidate[i]) {
      Serial.println("mismatch!");
      validationSuccessfull = false;
    } else {
      Serial.println("matched!");
    }
  }

  return validationSuccessfull;
#if 0
  for(int i = 0; i < PINLENGTH; i++) {
    sizeof(cardToValidate.pin)
  }

  char inputKey = pinpad.getKey();
  
  if (inputKey){
    Serial.println(customKey);
  }

  Serial.println("validatePin ...beendet");
#endif
}

// ******************** PROGRAM CODE ******************** //
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // for debugging
  SPI.begin();  // Enable the SPI interface for RFID reader
  RC522.init(); // Initialise the RFID reader
  
  servo.attach(SERVO_PIN);
  lockDoor();

  Serial.println("Leggo2!!!");


  allowedCards[1].serNum[0] = 0xA2;
  allowedCards[1].serNum[1] = 0x62;
  allowedCards[1].serNum[2] = 0x6C;
  allowedCards[1].serNum[3] = 0x03;
  allowedCards[1].serNum[4] = 0xAF;
  allowedCards[1].pin[0] = '1';
  allowedCards[1].pin[1] = '2';
  allowedCards[1].pin[2] = '1';
  allowedCards[1].pin[3] = '3';  

  allowedCards[2].serNum[0] = 0xC4;
  allowedCards[2].serNum[1] = 0x46;
  allowedCards[2].serNum[2] = 0xA5;
  allowedCards[2].serNum[3] = 0x4E;
  allowedCards[2].serNum[4] = 0x69;

  allowedCards[3].serNum[0] = 0xB3;
  allowedCards[3].serNum[1] = 0x73;
  allowedCards[3].serNum[2] = 0xF4;
  allowedCards[3].serNum[3] = 0x2C;
  allowedCards[3].serNum[4] = 0x18;
  
  stateMachine = StateHandler::CHECK_USER;  // go to start step
};

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();

  ledStatus.toggle();

  switch (stateMachine) {
    case StateHandler::CHECK_USER:
      waitForValidCard();
      if (isKnownCard(RC522.serNum)) {
        stateMachine = StateHandler::CHECK_PIN;
      } else {
        // Play failure sound
      }
      break;
    
    case StateHandler::CHECK_PIN:
      Serial.println("CHECK_PIN");
      if (validatePin(currentCard->pin)) {
        stateMachine = StateHandler::OPENING;
      } else {
        // Play failure sound
        stateMachine = StateHandler::CHECK_USER;
      }
      break;

    case StateHandler::OPENING:
      ledGreen.on();
      ledYellow.blink(currentMillis,0,200);
      ledRed.off();

      if (unlockDoor()) {
        stateMachine = StateHandler::UNLOCKED;
      }
      break;

    case StateHandler::UNLOCKED:
      ledGreen.on();
      ledYellow.off();
      ledRed.off();

      waitForValidCard();
      if (isKnownCard(RC522.serNum)) {
        stateMachine=StateHandler::CLOSING;
      };
      break;

    case StateHandler::CLOSING:
      ledYellow.blink(currentMillis,0,500);
      if (lockDoor()) {
        stateMachine = StateHandler::CHECK_USER;
      };
      break;

    case StateHandler::LOCKED:
      // check if necessary
      break;

    case StateHandler::THEFT:
      // keep locked and show alarm
      break;

    case StateHandler::SHOCK:
      // show different message on display?
      // maybe theft??
      break;
    
    default:
      break;
  };

  delay(100);   // let arduino chill for a second, to allow receiving all Serial bus data
};