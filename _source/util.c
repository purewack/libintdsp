#include "util.h"

int32_t lin_interp32(int32_t val1, int32_t val2, uint16_t numerator, uint16_t denom_shift){
  int32_t interp = (val2-val1)*numerator;
  interp >>= denom_shift;
  interp += val1;
  return interp;
}