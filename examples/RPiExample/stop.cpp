#include "Rover5.h"
#include <StateOfTheRobot.h>

DefineStates(Starting);

Rover5 bot;

state_func(Starting, [] {
    bot.begin();
    bot.move(0, 0);
    bot.end();
    stop_state_machine();
});
