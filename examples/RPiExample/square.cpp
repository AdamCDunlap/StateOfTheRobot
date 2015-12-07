#include "Rover5.h"
#include <StateOfTheRobot.h>
using namespace std::literals::chrono_literals;

Rover5 bot;

DefineStates(Starting, MoveForward, MoveRight, Done);

state_func(Starting, [] {
    bot.begin();

    set_state(MoveForward);
});

state_func(MoveForward, [] {
    bot.move(0, 80);
    next_substate();
}, wait(500ms), [] {
    set_state(MoveRight);
});

state_func(MoveRight, [] {
    bot.move(80, 0);
    next_substate();
}, wait(500ms), [] {
    set_state(Done);
});

state_func(Done, [] {
    bot.move(0,0);
    stop_state_machine();
});
