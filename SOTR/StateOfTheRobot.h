#ifndef STATEOFTHEROBOT_H
#define STATEOFTHEROBOT_H
#include <functional>
#include <vector>

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
    struct register_millis_func {
        register_millis_func(std::function<unsigned long()> millis_function);
    };

    extern std::function<unsigned long()> millis;
    // Map from state number to list of functions
    extern std::vector<std::vector<std::function<void()>>> state_setup_fns;
    extern std::vector<std::vector<std::function<void()>>> state_loop_fns;
    extern std::vector<std::vector<std::function<void()>>> state_cleanup_fns;

    bool stay_in_state();
}
#define set_error_func(ef) namespace _SOTR_Private { static register_err_func \
    err_func_registration(ef); }
#define state_func(st,...) namespace _SOTR_Private { static register_state_func \
    _SOTR_MACRO_CONCAT(state_func_defined_at_, __LINE__)(st,__VA_ARGS__); }
#define interrupt_func(trigger,fun) namespace _SOTR_Private { static \
    register_interrupt_func _SOTR_MACRO_CONCAT(interrupt_func_defined_at_, \
            __LINE__)(trigger,fun); }
#define set_millis(mf) namespace _SOTR_Private { static register_millis_func \
    millis_func_registration(mf); }

// Make the enum, then make the state_fns vector the right size so we can
// determine the number of states by asking for its size
#define DefineStates(...) enum { __VA_ARGS__, SOTR_LAST_STATE }; \
    std::vector<std::vector<std::function<void()>>> \
        _SOTR_Private::state_setup_fns(SOTR_LAST_STATE); \
    std::vector<std::vector<std::function<void()>>> \
        _SOTR_Private::state_loop_fns(SOTR_LAST_STATE); \
    std::vector<std::vector<std::function<void()>>> \
        _SOTR_Private::state_cleanup_fns(SOTR_LAST_STATE);

/* Defines a class for time. Nothing really special here
 */
class SOTR_Time {
public:
    SOTR_Time() = default;
    SOTR_Time(const SOTR_Time&) = default;
    SOTR_Time& operator=(const SOTR_Time&) = default;

    static SOTR_Time ms(unsigned long);
    static SOTR_Time us(unsigned long);

    unsigned long ms() const;
    unsigned long us() const;

    SOTR_Time& operator+=(SOTR_Time&);
    SOTR_Time operator+(SOTR_Time&) const;
    // Subtraction is defined, but time can only be positive. It is undefined to
    // subtract a bigger time from a smaller one
    SOTR_Time& operator-=(SOTR_Time&);
    SOTR_Time operator-(SOTR_Time&) const;
private:
    void normalizeUs();
    int us_;
    unsigned long ms_;
    friend SOTR_Time ms(unsigned long);
};

// Lets you write 500_ms to get a SOTR_Time object
SOTR_Time operator "" _ms(unsigned long long int t);

SOTR_Time time();

// Functions user can call in their state functions
SOTR_Time tm_in_state();
void wait(SOTR_Time);
void wait_for(std::function<bool()>);
void set_state(int s);

int state();
int prev_state();
int next_state();

/* A control structure that lets a piece of code be run every so often.
 * Currently only has a resolution of milliseconds, but could be adapted to
 * support microseconds
 */
// TODO: Call millis() less
#define every(t) for \
    (static unsigned long _SOTR_lasttime = -(t).ms(); \
    _SOTR_Private::millis() - _SOTR_lasttime >= (t).ms(); \
     _SOTR_lasttime = (_SOTR_Private::millis() - _SOTR_lasttime < (t).ms()) ? \
                        _SOTR_lasttime + (t).ms() : _SOTR_Private::millis())

#endif
