#include "Led.h"

Led::Led(int pin)
{
    this->pin = pin;
}

void Led::init()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    timeLastInteraction = 0;
    ledMax = 250;
    ledMin = 0;
};

void Led::test(){

};

void Led::update(unsigned long* pCurrentMillis){

};

void Led::on()
{
    digitalWrite(pin, HIGH);
};

void Led::off()
{
    digitalWrite(pin, LOW);
};

void Led::toggle()
{
    digitalWrite(pin, not digitalRead(pin));
};

void Led::fadeIn(unsigned long* pCurrentMillis, unsigned int time) {
    unsigned int fadetime = (int)(time / ledMax);
    unsigned long elapsedTime = (*pCurrentMillis - timeLastInteraction);

    if (elapsedTime >= fadetime ){
        light = light + ((int)elapsedTime/fadetime);
        if (light > ledMax) {light = ledMax;};
        analogWrite(pin,light);
        timeLastInteraction = *pCurrentMillis;
        Serial.println("LED fadeIn: " + (String)light);
    };
};

void Led::fadeOut(unsigned long* pCurrentMillis, unsigned int time) {
    unsigned int fadetime = (int)(time / ledMax);
    unsigned long elapsedTime = (*pCurrentMillis - timeLastInteraction);

    if (elapsedTime >= fadetime ){
        light = light - ((int)elapsedTime/fadetime);
        if (light > ledMax) {light = ledMax;};
        analogWrite(pin,light);
        timeLastInteraction = *pCurrentMillis;
        Serial.println("LED fadeOut: " + (String)light);
    };
};

void Led::setBrightness(int light)
{
    if (light > 255) {
        light = 255;
    }
    else if (light < 0) {
        light = 0;
    }

    analogWrite(pin, light);
};

int Led::getPin()
{
    return pin;
};

int Led::getBrightness() {
    //light = analogRead(pin);
    Serial.println("getBrightness: " + (String)analogRead(pin));
    return analogRead(pin);
};