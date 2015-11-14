#ifndef STATEOFTHEROBOT_H
#define STATEOFTHEROBOT_H
#include <string>
#include <functional>
#include <deque>

using _sotr_print_err_func = std::function<void(const char*)>;
_sotr_print_err_func _sotr_print_err;
struct _sotr_register_err_func {
    _sotr_register_err_func(_sotr_print_err_func err_print_func) {
        _sotr_print_err = err_print_func;
    }
};
#define set_error_func(ef) _sotr_register_err_func _err_func_def(ef)

using _sotr_flag_func = std::function<bool(void)>;
using _sotr_do_func = std::function<void(void)>;

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
    return _sotr_time::ms(t);
}

_sotr_time tm_in_state();

void wait(_sotr_time);

void wait_for(_sotr_flag_func);

// TODO
#define every(tm_inc) for(;;)

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

int _cur_state;
std::deque<int> _prev_states(1, 0);

void set_state(int s) {
    _prev_states.push_back(s);
    _cur_state = s;
}
int state() {
    return _cur_state;
}
int last_state() {
    return _prev_states.back();
}
struct _sotr_register_state_func{
    _sotr_register_state_func(int state, _sotr_do_func f);
};
struct _sotr_register_interrupt_func{
    _sotr_register_interrupt_func(_sotr_flag_func t, _sotr_do_func f);
};

#define _SOTR_MACRO_CONCAT2(x,y) x ## y
#define _SOTR_MACRO_CONCAT(x,y) _SOTR_MACRO_CONCAT2(x,y)
#define while_in(st,fun) _sotr_register_state_func _SOTR_MACRO_CONCAT(_state_func_defined_at_, __LINE__)(st,fun)
#define define_interrupt(trigger,fun) _sotr_register_interrupt_func _SOTR_MACRO_CONCAT(_interrupt_func_defined_at_, __LINE__)(trigger,fun)

using _sotr_millis_func_t = std::function<unsigned long(void)>;

// Maybe make this a compile error to not define it
_sotr_millis_func_t _sotr_millis = [] { _sotr_print_err("No millis function defined!"); return 0; };
struct _sotr_register_millis {
    _sotr_register_millis(_sotr_millis_func_t millis_function) {
        _sotr_millis = millis_function;
    }
};
#define set_millis(mf) _sotr_register_millis _millis_func_def(mf)


#include "StateOfTheRobot-private.h"

#endif
