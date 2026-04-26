#pragma once

typedef struct{
    float alpha;
    float value;
} low_pass_filter_t;

void lpf_init(low_pass_filter_t* f, float alpha);
float lpf_apply(low_pass_filter_t* f, float input);