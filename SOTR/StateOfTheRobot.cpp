#include "StateOfTheRobot.h"
#include <vector>
#include <functional>

using namespace _SOTR_Private;
using our_clock = std::chrono::steady_clock;

// Private data
static std::function<void(char*)> print_err_func;
//static std::deque<int> prev_states(2, 0);
static int cur_state_;
static int prev_state_;
static int next_state_;
static int cur_st_func_num_;
static our_clock::time_point enter_state_tm_;

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
static void checkInterrupts() {
    for(auto& TI : interrupt_fns()) {
        if (TI.first()) TI.second();
    }
}

int main() {
    next_state_ = 0;
    cur_state_ = 0;
    while(true) {
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
        while(next_state_ == -1) {
            cur_st_func_num_ = 0;
            for (state_fn & sf : stateFns) {
                checkInterrupts();
                if (sf.substate < sf.subfns.size()) {
                    sf.subfns[sf.substate]();
                }
                ++cur_st_func_num_;
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
void set_state(int s) {
    next_state_ = s;
}
void next_substate() {
    state_fn& sf = state_fns()[cur_state_][cur_st_func_num_];
    ++sf.substate;
    sf.enter_subst_tm = our_clock::now();
}
void next_substate(int n) {
    state_fn& sf = state_fns()[cur_state_][cur_st_func_num_];
    state_fns()[cur_state_][cur_st_func_num_].substate = n;
    sf.enter_subst_tm = our_clock::now();
}

our_clock::duration tm_in_substate() {
    return our_clock::now() -
        state_fns()[cur_state_][cur_st_func_num_].enter_subst_tm;
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
