#include "Led.h"

Led::Led(int pin)
{
    this->pin = pin;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    timeLastInteraction = 0;
    ledMax = 250;
    ledMin = 0;
};

void Led::test(){

};

void Led::update(unsigned long currentMillis){

};

void Led::on() {
    digitalWrite(pin, HIGH);
};

void Led::off() {
    digitalWrite(pin, LOW);
};

void Led::toggle() {
    digitalWrite(pin, not digitalRead(pin));
};

bool Led::blink(unsigned long currentMillis, unsigned int amount = 0, unsigned int duration = 100) {
    
    bool blinkDone;

    if (((currentMillis - timeLastInteraction) >= duration)){
        toggle();

        if (digitalRead(pin)) {
            blinkCounter = blinkCounter + 1;
        };
                
        timeLastInteraction = currentMillis;
    };

    blinkDone = (blinkCounter >= amount) and not (amount == 0);

    if (blinkDone and not (digitalRead(pin))) {
        blinkCounter = 0;
        blinkDone = false;
        timeLastInteraction = currentMillis;
        return true;
    };    
    
    return false;
};

bool Led::fadeIn(unsigned long currentMillis, unsigned int time) {
    unsigned int fadetime = (int)(time / ledMax);
    unsigned long elapsedTime = (currentMillis - timeLastInteraction);

    if (elapsedTime >= fadetime ){
        light = light + ((int)elapsedTime/fadetime);

        if (light > ledMax) {light = ledMax;};

        analogWrite(pin,light);
        timeLastInteraction = currentMillis;
    };
    return (light >= ledMax);
};

bool Led::fadeOut(unsigned long currentMillis, unsigned int time) {
    unsigned int fadetime = (int)(time / ledMax);
    unsigned long elapsedTime = (currentMillis - timeLastInteraction);

    if (elapsedTime >= fadetime ){
        light = light - ((int)elapsedTime/fadetime);

        if (light > ledMax) {light = ledMax;};

        analogWrite(pin,light);
        timeLastInteraction = currentMillis;
    };
    return (light <= ledMin);
};

void Led::setBrightness(int light) {
    if (light > 255) {
        light = 255;
    }
    else if (light < 0) {
        light = 0;
    };

    analogWrite(pin, light);
};

int Led::getPin() {
    return pin;
};

int Led::getBrightness() {
    Serial.println("getBrightness: " + (String)analogRead(pin));
    return analogRead(pin);
};