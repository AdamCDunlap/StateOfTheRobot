#include "StateOfTheRobot.h"
#include <vector>
#include <functional>

using namespace _SOTR_Private;
using our_clock = std::chrono::steady_clock;

// Private data
static std::function<void(char*)> print_err_func;
static int cur_state_;
static int prev_state_;
static int next_state_;
static int cur_st_func_num_;
static our_clock::time_point enter_state_tm_;
static bool stop_state_machine_;
static bool in_globals_;

struct global_fn : state_fn {
    bool enabled;
    global_fn(bool en,
             const std::vector<std::function<void()>>& subfns_)
        : state_fn(subfns_), enabled(en)
    {}
};

static std::vector<global_fn>&
global_fns() {
    static std::vector<global_fn>
        inner;
    return inner;
}

static std::vector<std::pair<std::function<bool()>, std::function<void()>>>&
interrupt_fns() {
    static std::vector<std::pair<std::function<bool()>, std::function<void()>>>
        inner;
    return inner;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////    Main Functions    /////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main() {
    interrupt_fns().shrink_to_fit();
    state_fns().shrink_to_fit();
    global_fns().shrink_to_fit();

    next_state_ = 0;
    cur_state_ = 0;
    stop_state_machine_ = false;
    while(!stop_state_machine_) {
        prev_state_ = cur_state_;
        cur_state_ = next_state_;
        next_state_ = -1;
        enter_state_tm_ = our_clock::now();

        std::vector<state_fn> & stateFns =
            state_fns()[cur_state_];
        for (state_fn & sf: stateFns) {
            sf.substate = 0;
            sf.enter_subst_tm = enter_state_tm_;
        }
        while(next_state_ == -1 && !stop_state_machine_) {
            cur_st_func_num_ = 0;
            in_globals_ = false;
            for (state_fn & sf : stateFns) {

                // Check interrupts really frequently
                for(auto& TI : interrupt_fns()) {
                    if (TI.first()) TI.second();
                }

                // Run the correct substate function
                if (sf.substate < sf.subfns.size()) {
                    sf.subfns[sf.substate]();
                }
                ++cur_st_func_num_;
            }
            in_globals_ = true;
            for(global_fn & gf : global_fns()) {
                if (gf.enabled && gf.substate < gf.subfns.size()) {
                    gf.subfns[gf.substate]();
                }
            }
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////////    Setters & Getters    ////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int state() {
    return cur_state_;
}
int prev_state() {
    return prev_state_;
}
int next_state() {
    return next_state_;
}
void stop_state_machine() {
    stop_state_machine_ = true;
}
void set_state(int s) {
    next_state_ = s;
}
void next_substate() {
    state_fn& sf = in_globals_? global_fns()[cur_st_func_num_]
                              : state_fns()[cur_state_][cur_st_func_num_];
    ++sf.substate;
    if (sf.substate >= sf.subfns.size()) {
        sf.substate = 0;
    }
    sf.enter_subst_tm = our_clock::now();
}
void next_substate(int n) {
    state_fn& sf = in_globals_? global_fns()[cur_st_func_num_]
                              : state_fns()[cur_state_][cur_st_func_num_];
    sf.substate = n;
    sf.enter_subst_tm = our_clock::now();
}
std::function<void()> wait(int n, std::chrono::steady_clock::duration d) {
    return wait_for(n, [d]{return tm_in_substate() > d;});
}
std::function<void()> wait(std::chrono::steady_clock::duration d) {
    return wait_for([d]{return tm_in_substate() > d;});
}

std::function<void()> wait_for(std::function<bool()> f) {
    return [f] {
        if (f()) {
            next_substate();
        }
    };
}
std::function<void()> wait_for(int n, std::function<bool()> f) {
    return [f,n] {
        if (f()) {
            next_substate(n);
        }
    };
}

our_clock::duration tm_in_substate() {
    state_fn& sf = in_globals_? global_fns()[cur_st_func_num_]
                              : state_fns()[cur_state_][cur_st_func_num_];
    return our_clock::now() - sf.enter_subst_tm;
}

our_clock::duration tm_in_state() {
    return our_clock::now() - enter_state_tm_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////    Registration    //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
_SOTR_Private::register_err_func::register_err_func(
        std::function<void(char*)> ef) {
    print_err_func = ef;
}
_SOTR_Private::register_state_func::register_state_func(
        int state,
        const std::vector<std::function<void()>>& fns) {
    state_fns()[state].emplace_back(fns);
}
_SOTR_Private::register_interrupt_func::register_interrupt_func(
        std::function<bool()> trigger,
        std::function<void()> func) {
    interrupt_fns().push_back(make_pair(trigger, func));
}
// TODO
_SOTR_Private::register_global_func::register_global_func(
        GlobalFuncStart en,
        const std::vector<std::function<void()>>& fns) {
    idx = global_fns().size();
    global_fns().emplace_back(en==GlobalFuncStart::enabled, fns);
}


global_func_handle::global_func_handle(size_t inIdx) : idx(inIdx) {}
void global_func_handle::enable(bool en) {
    global_fns()[idx].enabled = en;
}
void global_func_handle::disable() {
    global_fns()[idx].enabled = false;
}

