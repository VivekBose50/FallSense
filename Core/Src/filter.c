#include "filter.h"

void lpf_init(low_pass_filter_t* f, float alpha){
    f->alpha = alpha;
    f->value = 0;
}

float lpf_apply(low_pass_filter_t* f, float input){
    f->value = f->alpha * input + (1.0f - f->alpha) * f->value;
    return f->value;
}