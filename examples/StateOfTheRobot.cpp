#include "StateOfTheRobot.h"

static std::function<unsigned long(void)> millis;

_sotr_time tm_in_state() {
    // TODO
    return _sotr_time::ms(0);
}

void wait(_sotr_time tm) {
    // TODO
    // Somehow create a shadow state that knows how to get back to original
    
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////                          Time helpers                          ////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_millis(std::function<unsigned long(void)> millis_function) {
    millis = millis_function;
}

unsigned long _sotr_time::ms() const {
    return ms_;
}

unsigned long _sotr_time::us() const {
    return ms_*1000 + us_;
}

_sotr_time _sotr_time::ms(unsigned long ms) {
    _sotr_time ret;
    ret.ms_ = ms;
    return ret;
}

_sotr_time _sotr_time::us(unsigned long us) {
    _sotr_time ret;
    ret.ms_ = us/1000;
    ret.us_ = us%1000;
    return ret;
}


