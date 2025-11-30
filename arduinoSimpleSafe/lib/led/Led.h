#ifndef Led_h
#define Led_h

#include <Arduino.h>

enum stateOptions{FADEIN,FADEOUT,ON,OFF};  // state handler for update routine (mostly for fading)

class Led
{
private:
    unsigned int pin;   // pin-number of led on adruino board
    unsigned int time;  // time to take for operation in ms
    unsigned int light; // current brightness of LED
    unsigned int ledMin; // minimal value for PWM to regulate led brightness
    unsigned int ledMax;
    unsigned long timeLastInteraction;  // used for millis
    enum stateOptions state;  // state handler for update routine (mostly for fading)


public:
    Led(int);
    void test();            // only for test functionality, will be removed at any point in time
    void init();             // initialise object and set pin mode
    void update(unsigned long*);       // checks if objects needs to be updated (mostly for fading)
    void on();               // turns on LED
    void off();              // turns off LED
    void toggle();           // toggles LED from current status
    void fadeIn(unsigned long*, unsigned int);        // fades on LED in given time range
    void fadeOut(unsigned long*, unsigned int);       // fades off LED in given time range
    void setBrightness(int); // sets LED to given brightness, 0-255
    int getPin();            // returns configured pin of LED on arduino
    int getBrightness();     // returns the current brightness of LED

};

#endif