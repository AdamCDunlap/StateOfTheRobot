# StateOfTheRobot
An internal DSL in C++ to help define state machines and other robot logic helpers

# Usage
## Basic Usage

Using StateOfTheRobot is simple:
 * Include the header file. 
 * Define your states:
   * SOTR defines all of your states as an enum.
```
DefineStates(MyStartState, MySecondState, MyThirdState, etc);
```
 * Define state functions and interrupts.
   * To define a state function, you call the `state_func` macro, passing it
     first the state and second a function (usually a lambda) that will be
     repeatedly called by SOTR when the robot is in that state. You can define
     as many state functions for the same state as you want. They will get
     called in a round-robin style.
   * To define an interrupt, call the `interrupt_func` macro, passing it two
     functions. the first should return a bool. When that function returns true,
     the second function is called. Interrupt functions are called in every
     state, although they can do work dependant on the current state if you so
     desire.
 * Do not define a main() method: SOTR does that for you.
 * To change states, call `set_state()`, passing it the next state. Once that
   function returns, the state functions for that state will start to be called.
 * You can query the current state with `state()`, the previous state with
   `prev_state()`, and the next state (if you've called `set_state()` but the
   function has not yet returned) with `next_state()`.
 * The `tm_in_state()` function returns a duration representing how long the
   robot has been in the current state. This duration is a
   `std::chrono::steady_clock::duration`. The easiest way to use it is to have a
   line at the top of your file saying `using namespace
   std::literals::chrono_literals`, and then comparing the returned duration by
   simply typing `if (tm_in_state() > 300ms)` 

### Basic example
```
#include <SOTR/StateOfTheRobot.h>
using namespace std::literals::chrono_literals;

DefineStates(Start, GoNorth, GoSouth, Confused);

interrupt_func([]{return rand() % 400 == 0;}, [] {
    if (state() != Confused) {
        printf("Randomly got into interrupt from state %d\n", prev_state());
        set_state(Confused);
    }
});

state_func(Start, [] {
    set_state(GoNorth);
    printf("Starting up!\n");
});

state_func(GoNorth, [] {
    printf("Moving north.\n");
    if (tm_in_state() > 2s) {
        set_state(GoSouth);
    }
});
state_func(GoNorth, [] {
    printf("Also moving north!\n");
});

state_func(GoSouth, [] {
    printf("Moving south.\n");
    if (tm_in_state() > 2s) {
        set_state(GoNorth);
    }
});

state_func(Confused, [] {
    printf("I'm confused!\n");
}
```

## Advanced Usage
Substates are a useful way to get simple sequential logic in your robot without
having to define lots of states. Within each function declared with the
`state_func` macro, there is another number that is the substate. By default,
there is only one substate. To add additional substate functions, just pass them
as extra arguments to the `state_func` macro: you can have as many as you want.
Substate functions loop by default, and to move to the next substate, just call
`next_substate()`. You can also pass a number to `next_substate()` to specify
which substate to call next (0-indexed).

Fully-featured examples can be found in the examples directory. `printtest.cpp`
is a working example with substates that prints out what it's doing in an
ncurses display and switches states based off of key presses.
