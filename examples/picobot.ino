#include "Rover5.h"
#include "StateOfTheRobot.h"
#include <string>

Rover5 bot;

define_states(Start, GoNorth, GoSouth, Confused);
set_millis(millis);

// These angles are in milliradians
enum class Dir { NORTH=0, EAST=1571, SOUTH=3142, WEST=4712 };

bool senseDir(unsigned dir) {
    bot.setMeasDir(dir);
    wait_for(bot.atMeasDir());
    return bot.getDist() < 100;
}

define_interrupt(bot.getBumpSensor(), RISING_EDGE) {
    if (state != Confused) {
        Serial.println("Hit something! Was in state " + last_state());
    }
    set_state(Confused);
}

void initialize() {
    pinMode(13, OUTPUT);
}

void main_loop() {
    if (state == Start) {
        bot.move(0, 1);
        if (bot.senseDir(Dir::EAST)) {
            set_state(GoNorth);
        } else if (tm_in_state().ms() > 10000) {
            set_state(Confused);
        }
    } else if (state == GoNorth) {
        bot.move(1, 0);
        if (bot.senseDir(Dir::NORTH)) {
            set_state(GoSouth);
        } else if (tm_in_state().ms() > 10000) {
            set_state(Confused);
        }
    } else if (state == GoSouth) {
        bot.move(-1, 0);
        if (bot.senseDir(Dir::SOUTH)) {
            set_state(GoNorth);
        } else if (tm_in_state().ms() > 10000) {
            set_state(Confused);
        }
    } else if (state == Confused) {
        Serial.println("I'm confused! I got here from state " + last_state());
        digitalWrite(13, LOW);
        wait(500_ms);
        digitalWrite(13, HIGH);
        wait(500_ms);
    } else if (state == Panic) {
        every(100_ms) {
             Serial.println("BEAR!");
        }
    }
}

void checkForBears() {
    scanBearSensor();
    if (bot.seeBear()) {
        state = Panic;
    }
}

void scanBearSensor() {
    bot.moveBearSensor(EAST);
    wait_for(bot.bearSensorInPlace());
    bot.moveBearSensor(WEST);
    wait_for(bot.bearSensorInPlace());
}

void background_loop() {
    if (state == GoNorth) {
        // While moving north, we should check for polar bears
        checkForBears();
    }
}
