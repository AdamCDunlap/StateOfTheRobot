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

int32_t startDists[4];
state_func(MoveRight, [] {
    bot.updateEncoders();
    bot.getDists(startDists);
    bot.move(80, 0);
    next_substate();
}, [] {
    int32_t dists[4];
    bot.updateEncoders();
    bot.getDists(dists);
    if (dists[0]-startDists[0] >= 12000) {
        next_substate();
    }
}, [] {
    set_state(Done);
});

state_func(Done, [] {
    bot.move(0,0);
    bot.end();
    stop_state_machine();
});
