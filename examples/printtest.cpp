#include <SOTR/StateOfTheRobot.h>
#include <string>
#include <ncurses.h>
using namespace std::chrono;
using namespace std::literals::chrono_literals;

DefineStates(Start, GoNorth, GoSouth, Confused, Panic);

WINDOW* win;

// Sees if the user has typed the given character. Doesn't modify the character
// queue if they haven't
bool isCh(char desired) {
    int c = getch();
    if (c == ERR) return false;
    if (c == desired) return true;
    ungetch(c);
    return false;
}

global_func(GlobalFuncStart::enabled, [] {
    every(1400ms) {
        printw("In the global function\n");
    }
});

interrupt_func([]{return state() > Start && isCh('=');}, [] {
    if (state() != Confused) {
        printw("Saw '='! Was in state %d\n", prev_state());
    }
    set_state(Confused);
});

state_func(Start, [] {
    win = initscr();
    scrollok(win, TRUE);
    timeout(0);
    noecho();
    set_state(GoNorth);
    printw("Pressing = triggers an interrupt\n");
    });

state_func(GoNorth, [] {
    every(1s) {
        printw("Moving north. Press n to move on\n");
        printw("Been in this state for %lu\n", duration_cast<milliseconds>(tm_in_state()).count());
    }
    if (isCh('n')) {
        set_state(GoSouth);
    } else if (tm_in_state() > 10s) {
        set_state(Confused);
    }
});

state_func(GoSouth, [] {
    every(1s) {
        printw("Moving south. Press n to move on\n");
    }
    if (isCh('n')) {
        set_state(GoNorth);
    } else if (tm_in_state() > 10s) {
        set_state(Confused);
    }
});

state_func(Confused, [] {
    printw("Entering confused state, prev state is %d\n", prev_state());
    next_substate();
}, [] {
    printw("Tick\n");
    next_substate();
},
    wait(500ms),
[] {
    printw("Tock\n");
    next_substate();
},
    wait(1, 500ms)
);

state_func(Panic, [] {
    every(1s) {
         printw("BEAR!\n");
    }
});

// Scan the bear sensor when we're going north
state_func(GoNorth, [] {
    printw("Move sensor right. Press 0 to stop\n");
    next_substate();
},
    wait_for(std::bind(isCh, '0')),
[] {
    printw("Move sensor left. Press 9 to stop\n");
    next_substate();
},
    wait_for(std::bind(isCh, '9'))
);

state_func(GoNorth, [] {
    every(1s) {
        printw("Press b if there's a bear\n");
    }
    if (isCh('b')) {
        set_state(Panic);
    }
});
