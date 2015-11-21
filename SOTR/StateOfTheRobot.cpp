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
our_clock::duration tm_in_state() {
    return our_clock::now() - enter_state_tm_;
}
void wait(our_clock::duration) {
    // TODO
}
void wait_for(std::function<bool()>) {

    // TODO
}

static void checkInterrupts() {
    for(auto& TI : interrupt_fns()) {
        if (TI.first()) TI.second();
    }
}

int main() {
    next_state_ = 0;
    while(true) {
        cur_state_ = next_state_;
        next_state_ = -1;
        std::vector<std::function<void(void)>> & setupFns =
            state_setup_fns()[cur_state_];
        std::vector<std::function<void(void)>> & loopFns =
            state_loop_fns()[cur_state_];
        std::vector<std::function<void(void)>> & cleanupFns =
            state_cleanup_fns()[cur_state_];
        enter_state_tm_ = our_clock::now();
        for (auto & setupFn : setupFns) {
            checkInterrupts();
            setupFn();
        }
        while(next_state_ == -1) {
            for (auto & loopFn : loopFns) {
                checkInterrupts();
                loopFn();
            }
        }
        for (auto & cleanupFn : cleanupFns) {
            checkInterrupts();
            cleanupFn();
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
        std::function<void()> loop) {
    state_loop_fns()[state].push_back(loop);
}
_SOTR_Private::register_state_func::register_state_func(
        int state,
        std::function<void()> setup,
        std::function<void()> loop)
    : register_state_func(state, loop) {
    state_setup_fns()[state].push_back(setup);
}
_SOTR_Private::register_state_func::register_state_func(
        int state,
        std::function<void()> setup,
        std::function<void()> loop,
        std::function<void()> cleanup)
    : register_state_func(state, setup, loop) {

    state_cleanup_fns()[state].push_back(cleanup);
}
_SOTR_Private::register_interrupt_func::register_interrupt_func(
        std::function<bool()> trigger,
        std::function<void()> func) {
    interrupt_fns().push_back(make_pair(trigger, func));
}
