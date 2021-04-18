#ifndef NUMBER_UTIL_H
#define NUMBER_UTIL_H

static int NUMBER_UTIL_INT_MAX = 0xFFFFFFFF / 2;
static long NUMBER_UTIL_LONG_MAX = 0xFFFFFFFFFFFFFFFF / 2;

long NumberUtil_NextPowerOf_2(int input);

long NumberUtil_NextPrime(int input);

#endif