#ifndef STATEOFTHEROBOT_H
#define STATEOFTHEROBOT_H
#include <functional>
#include <vector>
#include <chrono>

// Macros to concatenate tokens in a macro
// Lets you do _SOTR_MACRO_CONCAT(sup, __LINE__)
// to get "sup42" as a C++ token
#define _SOTR_MACRO_CONCAT2(x,y) x ## y
#define _SOTR_MACRO_CONCAT(x,y) _SOTR_MACRO_CONCAT2(x,y)

/* The user defines their functions in the top-level part, so they can't be
 * normal functions since functions can't be called outside other functions.
 * Instead, we'll make all the registrations of state functions and such be
 * calls to constructors for classes, and the constructors will do the work. We
 * then define macros so that they look like function calls. These macros create
 * unique names for the objects whos constructors are doing the work by using
 * the __LINE__ macro, which is the current line.
 *
 * This namespace is for things that need to be "visible" to compiler while
 * compiling the user's code, but the user should not interact with.
 */
namespace _SOTR_Private {
    // User calls constructor which registers the function
    struct register_err_func {
        register_err_func(std::function<void(char*)>);
    };
    struct register_state_func {
        register_state_func (int state,
                const std::vector<std::function<void()>>& fns);
    };
    struct register_interrupt_func {
        register_interrupt_func(std::function<bool(void)>,
                std::function<void()>);
    };
    // Define singleton global vectors to store the state functions
    // parameter is only used the first time they are called
    // Map from state number to list of functions
    //
    // Function that returns the singleton map from state number to list of
    // "state functions". A state function is a substate number, the time it
    // last switched substates, and a list of substate functions
    struct state_fn {
        std::vector<std::function<void()>> subfns;
        std::chrono::steady_clock::time_point enter_subst_tm;
        unsigned substate;
        state_fn(const std::vector<std::function<void()>>& subfns_)
            : subfns(subfns_)
        {}
    };
    std::vector<std::vector<state_fn>>& state_fns();
}
#define set_error_func(ef) namespace _SOTR_Private { static register_err_func \
    err_func_registration(ef); }
#define state_func(st,...) namespace _SOTR_Private { static register_state_func \
    _SOTR_MACRO_CONCAT(state_func_defined_at_, __LINE__)(st,{__VA_ARGS__}); }
#define interrupt_func(trigger,fun) namespace _SOTR_Private { static \
    register_interrupt_func _SOTR_MACRO_CONCAT(interrupt_func_defined_at_, \
            __LINE__)(trigger,fun); }

// Make the enum, then make the state_fns vector the right size so we can
// determine the number of states by asking for its size
#define DefineStates(...) enum { __VA_ARGS__, _SOTR_LAST_STATE }; \
    std::vector<std::vector<_SOTR_Private::state_fn>>& \
    _SOTR_Private::state_fns() { \
        static std::vector<std::vector<_SOTR_Private::state_fn>> \
            inner(_SOTR_LAST_STATE); \
       return inner; \
    }

// Functions user can call in their state functions
std::chrono::steady_clock::duration tm_in_state();
std::chrono::steady_clock::duration tm_in_substate();
void set_state(int s);
void next_substate();
void next_substate(int n);


int state();
int prev_state();
int next_state();

/* A control structure that lets a piece of code be run every so often.
 * Currently only has a resolution of milliseconds, but could be adapted to
 * support microseconds
 */
// TODO: Use helper class to call now() less often since now() is expensive
#define every(t) for \
    (std::chrono::steady_clock::time_point _SOTR_lasttime = \
        std::chrono::steady_clock::time_point::min(); \
     std::chrono::steady_clock::now() - _SOTR_lasttime >= (t); \
     _SOTR_lasttime = (std::chrono::steady_clock::now() - _SOTR_lasttime < (t)?\
                              _SOTR_lasttime + (t) \
                            : std::chrono::steady_clock::now()))

#endif
