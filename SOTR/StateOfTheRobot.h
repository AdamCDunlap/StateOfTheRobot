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
        register_state_func (int state, std::function<void()> loop);
        register_state_func (int state, std::function<void()> setup,
                std::function<void()> loop);
        register_state_func (int state, std::function<void()> setup,
                std::function<void()> loop, std::function<void()> cleanup);
    };
    struct register_interrupt_func {
        register_interrupt_func(std::function<bool(void)>, std::function<void()>);
    };
    // Define singleton global vectors to store the state functions
    // parameter is only used the first time they are called
    // Map from state number to list of functions
    std::vector<std::vector<std::function<void()>>>& state_setup_fns();
    std::vector<std::vector<std::function<void()>>>& state_loop_fns();
    std::vector<std::vector<std::function<void()>>>& state_cleanup_fns();

    bool stay_in_state();
}
#define set_error_func(ef) namespace _SOTR_Private { static register_err_func \
    err_func_registration(ef); }
#define state_func(st,...) namespace _SOTR_Private { static register_state_func \
    _SOTR_MACRO_CONCAT(state_func_defined_at_, __LINE__)(st,__VA_ARGS__); }
#define interrupt_func(trigger,fun) namespace _SOTR_Private { static \
    register_interrupt_func _SOTR_MACRO_CONCAT(interrupt_func_defined_at_, \
            __LINE__)(trigger,fun); }

// Make the enum, then make the state_fns vector the right size so we can
// determine the number of states by asking for its size
// TODO: We don't want to set the size here, just the capacity :/
#define DefineStates(...) enum { __VA_ARGS__, _SOTR_LAST_STATE }; \
    std::vector<std::vector<std::function<void()>>>& \
    _SOTR_Private::state_setup_fns() { \
       static std::vector<std::vector<std::function<void()>>> \
        inner(_SOTR_LAST_STATE); \
       return inner; \
    } \
std::vector<std::vector<std::function<void()>>>& \
    _SOTR_Private::state_loop_fns() { \
       static std::vector<std::vector<std::function<void()>>> \
        inner(_SOTR_LAST_STATE); \
       return inner; \
    } \
std::vector<std::vector<std::function<void()>>>& \
    _SOTR_Private::state_cleanup_fns() { \
        static std::vector<std::vector<std::function<void()>>> \
            inner(_SOTR_LAST_STATE); \
        return inner; \
    }

// Functions user can call in their state functions
std::chrono::steady_clock::duration tm_in_state();
void wait(std::chrono::steady_clock::duration);
void wait_for(std::function<bool()>);
void set_state(int s);

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
