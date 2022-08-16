#pragma once
#include "types.h"

/*
    return interpolated sample value as between val1 and val2 denoted by percentage of num/(1<<denom_shift)
    usual values = 8 for denom_shift with 256 points of interpolation range
*/
int32_t lin_interp32(int32_t val1, int32_t val2, uint16_t numerator, uint16_t denom_shift);

/*
    limit value of val inbetween low and high constraints
*/
int32_t clamp32(int32_t *val, int32_t low, int32_t high);