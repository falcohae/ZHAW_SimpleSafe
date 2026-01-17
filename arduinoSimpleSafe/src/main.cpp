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
#define SERVO_DELAY 33
#define SERVO_SIMULATE false
#define PINLENGTH 4

struct rfidCard {
  unsigned char serNum[5];
  unsigned char pin[4];
  bool allowClose;
  bool allowOpen;
};

struct rfidCard allowedCards[4];
rfidCard *currentCard;

// Objects
Led ledRed(4);
Led ledYellow(3);
Led ledGreen(2);
Led buzzer(7);  // LED object can be used for buzzer to enhance handling
Led ledStatus(LED_BUILTIN);
Led ledTest(12);

// RFID Reader
RFID RC522(SDA_DIO, RESET_DIO);

// Serco
Servo servo;

// Keypad
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

// Buzzer
#define BUZZERPIN 7

// Display
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C display(0x27,20,4);

void clearDisplayLine(unsigned int lineNumber) {
  display.setCursor(0, lineNumber);
  display.print("                    ");
}

void beep(unsigned int duration) {
  digitalWrite(BUZZERPIN, HIGH);
  delay(duration);
  digitalWrite(BUZZERPIN, LOW);
}

void beepOk() {
  beep(100);
  delay(50);
  beep(100);
  delay(50);
  beep(100);
}

void beepActionFinished() {
  beep(50);
  delay(50);
  beep(50);
}

void beepButtonPress() {
  beep(50);
}

void beepNotOk() {
  beep(300);
  delay(50);
  beep(300);
  delay(50);
  beep(300);
}


// ******************** FUNCTION CODE ******************** //

bool cardHasPin(unsigned char CardSerial[5]) {
  return CardSerial[0] != 0;
}

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
  clearDisplayLine(2);
  display.setCursor(4, 2);
  display.print("Unlocking...");
  #if (SERVO_SIMULATE)    // only evaluated at compile time
    Serial.println("Entriegle Tuere... Simulation.");
    delay(500);
    return true;
  #else
    int servostartposition = servo.read();
    for(int i = servostartposition; i < SERVO_ENTRIEGELT; i++) {
      ledYellow.blink(currentMillis, 100, 0);
      servo.write(i);
      delay(SERVO_DELAY);
    };
    ledYellow.off();
    servo.write(SERVO_ENTRIEGELT);
    clearDisplayLine(2);
    display.setCursor(6, 2);
    display.print("Unlocked");

  beepActionFinished();
  Serial.println("Entriegle Tuere... abgeschlossen.");
    return (servo.read() >= SERVO_ENTRIEGELT);
  #endif
};

bool lockDoor() {
  Serial.println("Verriegle Tuere...");
  clearDisplayLine(2);
  display.setCursor(5, 2);
  display.print("Locking...");
  #if (SERVO_SIMULATE)
    Serial.println("Verriegle Tuere... Simulation.");
  #else
    int servostartposition = servo.read();
    for(int i = servostartposition; i > SERVO_VERRIEGELT; i--) {
      servo.write(i);
      ledYellow.blink(currentMillis, 100, 0);
      delay(SERVO_DELAY);
    };
    ledYellow.off();
    servo.write(SERVO_VERRIEGELT);
    clearDisplayLine(2);
    display.setCursor(7, 2);
    display.print("LOCKED");
  #endif
    Serial.println("Verriegle Tuere... abgeschlossen.");
  beepActionFinished();
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
      return; // FIXME
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
    beepButtonPress();
    delay(200); // debounce
    return buttonPressed;
}


bool validatePin(unsigned char pinToValidate[]) {
  Serial.println("validatePin");

  clearDisplayLine(2);
  display.setCursor(2, 2);
  display.print("Enter PIN: ");

  Serial.print("Validating against PIN ");



  bool validationSuccessfull = true;

  for(int i = 0; i < PINLENGTH; i++) {
    Serial.print(pinToValidate[i], HEX);
    Serial.print("|");
  }
  Serial.println();

  for(int i = 0; i < PINLENGTH; i++) {
    char buttonPressed = readButtonPress();
    display.setCursor(13 + i, 2);
    display.print("*");
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
  
  Serial.println("Leggo2!!!");

  allowedCards[0].serNum[0] = 0x0D;
  allowedCards[0].serNum[1] = 0x62;
  allowedCards[0].serNum[2] = 0xC9;
  allowedCards[0].serNum[3] = 0x01;
  allowedCards[0].serNum[4] = 0xA7;
  allowedCards[0].pin[0] = '1';
  allowedCards[0].pin[1] = '2';
  allowedCards[0].pin[2] = '1';
  allowedCards[0].pin[3] = '3';
  allowedCards[0].allowOpen = true;
  allowedCards[0].allowClose = false;

  allowedCards[1].serNum[0] = 0xA2;
  allowedCards[1].serNum[1] = 0x62;
  allowedCards[1].serNum[2] = 0x6C;
  allowedCards[1].serNum[3] = 0x03;
  allowedCards[1].serNum[4] = 0xAF;
  allowedCards[1].pin[0] = 'A';
  allowedCards[1].pin[1] = 'B';
  allowedCards[1].pin[2] = 'A';
  allowedCards[1].pin[3] = 'C';
  allowedCards[1].allowOpen = true;
  allowedCards[1].allowClose = false;

  allowedCards[2].serNum[0] = 0xC4;
  allowedCards[2].serNum[1] = 0x46;
  allowedCards[2].serNum[2] = 0xA5;
  allowedCards[2].serNum[3] = 0x4E;
  allowedCards[2].serNum[4] = 0x69;
  allowedCards[2].allowClose = true;
  allowedCards[2].allowOpen = false;

  allowedCards[3].serNum[0] = 0x65;
  allowedCards[3].serNum[1] = 0x2E;
  allowedCards[3].serNum[2] = 0x2D;
  allowedCards[3].serNum[3] = 0x03;
  allowedCards[3].serNum[4] = 0x65;
  allowedCards[3].allowClose = true;
  allowedCards[3].allowOpen = false;

  display.init();
  display.backlight();

  display.backlight();
  display.setCursor(2,0);
  display.print("ZHAW SimpleSafe");
  display.setCursor(4,2);
  display.print("Starting...");

  servo.write(SERVO_VERRIEGELT);
  servo.attach(SERVO_PIN);

  pinMode(BUZZERPIN, OUTPUT);

  beepOk();

  stateMachine = StateHandler::CLOSING;  // go to start step
};

void debugPrintCurrentCard() {
  Serial.print("Current card is ");
  for(int i = 0; i < 5; i++) {
    Serial.print(currentCard->serNum[i], HEX);
  }
  Serial.println();
  if(currentCard->allowOpen) {
    Serial.println("Allowed to open");
  }
  if(currentCard->allowClose) {
    Serial.println("Allowed to close");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();

  ledStatus.toggle();

  switch (stateMachine) {
    case StateHandler::CHECK_USER:
      waitForValidCard();
      if(isKnownCard(RC522.serNum) and currentCard->allowOpen) {
        stateMachine = StateHandler::CHECK_PIN;
      } else {
        beepNotOk();
      }
      debugPrintCurrentCard();
      break;
    
    case StateHandler::CHECK_PIN:
      Serial.println("CHECK_PIN");
      beepButtonPress();
      ledYellow.on();
      if (validatePin(currentCard->pin)) {
        beepOk();
        stateMachine = StateHandler::OPENING;
      } else {
        clearDisplayLine(2);
        display.setCursor(4,2);
        display.print("PIN is wrong");
        beepNotOk();
        delay(1000);
        clearDisplayLine(2);
        display.setCursor(7, 2);
        display.print("LOCKED");
        stateMachine = StateHandler::CHECK_USER;
      }
      ledYellow.off();
      debugPrintCurrentCard();
      break;

    case StateHandler::OPENING:
      if (unlockDoor()) {
        stateMachine = StateHandler::UNLOCKED;
      }
      break;

    case StateHandler::UNLOCKED:
      ledGreen.on();
      ledYellow.off();
      ledRed.off();

      waitForValidCard();
      debugPrintCurrentCard();
      if (isKnownCard(RC522.serNum) and currentCard->allowClose) {
        beepOk();
        stateMachine=StateHandler::CLOSING;
      } else {
        beepNotOk();
      };
      break;

    case StateHandler::CLOSING:
      if (lockDoor()) {
        stateMachine = StateHandler::LOCKED;
      };
      break;

    case StateHandler::LOCKED:
      ledRed.on();
      ledGreen.off();
      stateMachine=StateHandler::CHECK_USER;
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