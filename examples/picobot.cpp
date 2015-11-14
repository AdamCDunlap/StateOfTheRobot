#include "Rover5.h"
#include <SOTR/StateOfTheRobot.h>
#include <string>

#ifndef ARDUINO
// From Ardino
unsigned long millis();
class MySerial {
public:
    void println(int);
    void println(const char* s);
    void print(const char* s);
};
MySerial Serial;
void pinMode(int, int);
void digitalWrite(int, int);
#define OUTPUT 0
#define HIGH 1
#define LOW 0
#endif

Rover5 bot;

DefineStates(Start, GoNorth, GoSouth, Confused, Panic);

set_millis(millis);

// These angles are in milliradians
enum Dir { NORTH=0, EAST=1571, SOUTH=3142, WEST=4712 };

bool senseDir(unsigned dir) {
    bot.setMeasDir(dir);
    wait_for([]{return bot.atMeasDir();});
    return bot.getDist() < 100;
}

interrupt_func([]{return bot.getBumpSensor();}, [] {
    if (state() != Confused) {
        
        Serial.print("Hit something! Was in state ");
        Serial.println(prev_state());
    }
    set_state(Confused);
});

state_func(Start, [] {
    pinMode(13, OUTPUT);
    bot.Run(0, 1);
    if (bot.senseDir(EAST)) {
        set_state(GoNorth);
    } else if (tm_in_state().ms() > 10000) {
        set_state(Confused);
    }
});

state_func(GoNorth, [] {
    bot.Run(1, 0);
    if (bot.senseDir(NORTH)) {
        set_state(GoSouth);
    } else if (tm_in_state().ms() > 10000) {
        set_state(Confused);
    }
});

state_func(GoSouth, [] {
    bot.Run(-1, 0);
    if (bot.senseDir(SOUTH)) {
        set_state(GoNorth);
    } else if (tm_in_state().ms() > 10000) {
        set_state(Confused);
    }
});

state_func(Confused, [] {
        Serial.print("I'm confused! I got here from state ");
        Serial.println(prev_state());
    }, [] {
        digitalWrite(13, LOW);
        wait(500_ms);
        digitalWrite(13, HIGH);
        wait(500_ms);
    }
);

state_func(Panic, [] {
    every(100_ms) {
         Serial.println("BEAR!");
    }
});

void scanBearSensor() {
    bot.moveBearSensor(EAST);
    wait_for([]{return bot.bearSensorInPlace();});
    bot.moveBearSensor(WEST);
    wait_for([]{return bot.bearSensorInPlace();});
}

void checkForBears() {
    scanBearSensor();
    if (bot.seeBear()) {
        set_state(Panic);
    }
}

state_func(GoNorth, [] {
    // While moving north, we should check for polar bears
    checkForBears();
});
