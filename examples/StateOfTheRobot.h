#ifndef STATEOFTHEROBOT_H
#define STATEOFTHEROBOT_H
#include <string>
#include <functional>

class _sotr_time {
public:
    _sotr_time(const _sotr_time&) = default;
    _sotr_time& operator=(const _sotr_time&) = default;

    static _sotr_time ms(unsigned long);
    static _sotr_time us(unsigned long);

    unsigned long ms() const;
    unsigned long us() const;
private:
    _sotr_time();
    unsigned us_;
    unsigned long ms_;
    friend _sotr_time ms(unsigned long);
};

_sotr_time operator "" _ms(unsigned long long int t) {
    return ms(t);
}

_sotr_time tm_in_state();

void wait(_sotr_time);

template<class WaitFunc>
void wait_for(WaitFunc);

class dumb_state_name_wrapper {
    static std::string get(int e) {
        return std::string("State ") + std::to_string(e);
    }
};

template<typename CEnum, typename enum_name_wrapper>
class _type_safe_enum {
    CEnum cEnum_;
public:
    _type_safe_enum() = default;
    _type_safe_enum(const CEnum& cEnum)
        : cEnum_(cEnum)
    {
    }
    _type_safe_enum& operator=(const CEnum& cEnum) {
        cEnum_ = cEnum;
    }
    operator CEnum() const { return cEnum_; }
    operator std::string() const { return enum_name_wrapper::get(cEnum_); }
};

#define define_states(...) int _DefinedStates; \
    enum _user_defined_states { __VA_ARGS__ }; \
    using State = _type_safe_enum<_user_defined_states, dumb_state_name_wrapper>; \
    State state; \
    std::deque<State> _prev_states(1, 0); \
    void set_state(State s) { \
       \
        _prev_states.push_back(s); \
        state = _prev_states; \
    } \
    State cur_state() { \
        return state; \
    } \
    State last_state() { \
        return _prev_states.back()\
    }

// TODO: Define these functions

void set_millis(std::function<unsigned long(void)> millis_function);

#include "StateOfTheRobot-private.h"

#endif
