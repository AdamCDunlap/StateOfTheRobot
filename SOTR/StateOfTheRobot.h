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

enum class GlobalFuncStart { disabled, enabled };
namespace _SOTR_Private { struct register_global_func; }
class global_func_handle {
public:
    void enable(bool en=true);
    void disable();

    global_func_handle() = delete;
private:
    global_func_handle(size_t inIdx);
    friend class _SOTR_Private::register_global_func;
    size_t idx;
};

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
    struct register_debug_func {
        register_debug_func(std::function<void(char*)>);
    };
    struct register_state_func {
        register_state_func (int state,
                const std::vector<std::function<void()>>& fns);
    };
    struct register_global_func {
        register_global_func (GlobalFuncStart en,
                const std::vector<std::function<void()>>& fns);
        global_func_handle get_handle() {
            return global_func_handle(idx);
        }
    private:
        size_t idx;
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

    class every_helper {
        const std::chrono::steady_clock::duration dur_;
        std::chrono::steady_clock::time_point prev_time_;
        bool first_call_;
    public:
        every_helper(std::chrono::steady_clock::duration dur)
            : dur_(dur),
              prev_time_(std::chrono::steady_clock::time_point::min()),
              first_call_(true)
        {
        }
        bool condition() {
            auto now = std::chrono::steady_clock::now();
            auto diff = now - prev_time_;

            if (diff >= dur_ || first_call_) {
                if (diff >= 2*dur_ || first_call_) {
                    // We've missed a point, so re-sync to current time
                    prev_time_ = now;
                    first_call_ = false;
                } else {
                    prev_time_ += dur_;
                }
                return true;
            }
            return false;
        }
    };
}
#define set_error_func(ef) namespace _SOTR_Private { static register_err_func \
    err_func_registration(ef); }
#define set_debug_func(df) namespace _SOTR_Private { static register_debug_func \
    debug_func_registration(df); }
#define global_func(en,...) _SOTR_Private::register_global_func \
                                (en,{__VA_ARGS__}).get_handle();
//#define global_func(en,...) namespace _SOTR_Private { static \
//    register_global_func _SOTR_MACRO_CONCAT(global_func_defined_at_, \
//            __LINE__)(en,{__VA_ARGS__}); }, \
//    _SOTR_MACRO_CONCAT(global_func_defined_at_, __LINE__).get_handle()
#define state_func(st,...) namespace _SOTR_Private { static register_state_func \
    _SOTR_MACRO_CONCAT(state_func_defined_at_, __LINE__)(st,{__VA_ARGS__}); }
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

// Auto-defined substate functions that do nothing but wait for the condition
// to be true before advancing substates
std::function<void()> wait_for(int n, std::function<bool()> f);
std::function<void()> wait_for(std::function<bool()> f);
std::function<void()> wait(int n, std::chrono::steady_clock::duration d);
std::function<void()> wait(std::chrono::steady_clock::duration d);


int state();
int prev_state();
int next_state();
void stop_state_machine();

/* A control structure that lets a piece of code be run every so often.
 * Currently only has a resolution of milliseconds, but could be adapted to
 * support microseconds
 */
#define every(t) for \
    (static _SOTR_Private::every_helper _SOTR_No_Nested_Everys(t); \
     _SOTR_No_Nested_Everys.condition();)

#endif

