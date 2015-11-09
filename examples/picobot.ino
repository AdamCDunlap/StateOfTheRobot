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

void setup() {
    pinMode(13, OUTPUT);
}

void loop() {
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
    }
}
