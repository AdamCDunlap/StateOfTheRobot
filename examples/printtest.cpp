#include <SOTR/StateOfTheRobot.h>
#include <string>
#include <ncurses.h>
using namespace std::chrono;
using namespace std::literals::chrono_literals;

#ifdef NO_CURSES
#define printw printf
#endif

DefineStates(Start, GoNorth, GoSouth, Confused, Panic);

// These angles are in milliradians
enum Dir { NORTH=0, EAST=1571, SOUTH=3142, WEST=4712 };

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

interrupt_func([]{return state() > Start && isCh('=');}, [] {
    if (state() != Confused) {
        printw("Saw '='! Was in state %d\n", prev_state());
    }
    set_state(Confused);
});

state_func(Start, [] {
#ifndef NO_CURSES
    win = initscr();
    scrollok(win, TRUE);
    timeout(0);
#endif
    set_state(GoNorth);
    printw("Pressing = triggers an interrupt\n");
    });

state_func(GoNorth, [] {
    printw("Moving north. Press n to move on\n");
    printw("Been in this state for %lu\n", duration_cast<milliseconds>(tm_in_state()).count());
    if (isCh('n')) {
        set_state(GoSouth);
    } else if (duration_cast<milliseconds>(tm_in_state()).count() > 10000) {
        set_state(Confused);
    }
});

state_func(GoSouth, [] {
    printw("Moving south. Press n to move on\n");
    if (isCh('n')) {
        set_state(GoNorth);
    } else if (duration_cast<milliseconds>(tm_in_state()).count() > 10000) {
        set_state(Confused);
    }
});

state_func(Confused, [] {
        printw("Entering confused state, prev state is %d\n", prev_state());
    }, [] {
        printw("Tick\n");
        wait(500ms);
        printw("Tock\n");
        wait(500ms);
    }
);

state_func(Panic, [] {
    every(100ms) {
         printw("BEAR!\n");
    }
});

void scanBearSensor() {
    printw("Move sensor right. Press 0 to stop\n");
    wait_for([]{return isCh('0');});
    printw("Move sensor left. Press 9 to stop\n");
    wait_for([]{return isCh('9');});
}

void checkForBears() {
    scanBearSensor();
}

state_func(GoNorth, [] {
    printw("Checking for bears\n");
    // While moving north, we should check for polar bears
    checkForBears();
});
