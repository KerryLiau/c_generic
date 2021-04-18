#include "math.h"
#include "number_util.h"
#include "common_util.h"

// ================================================================================
// Private Properties
// ================================================================================
static long _Next_PowerOf2(int input, int pow_number)
{
    int result = (long) pow(2, pow_number);
    if (result > input)
    {
        return result;
    }
    if (result == NUMBER_UTIL_INT_MAX + 1)
    {
        // Integer 的最大正整數
        return NUMBER_UTIL_INT_MAX;
    }
    return _Next_PowerOf2(input, pow_number + 1);
}

static bool _IsPrime(int input)
{
    if (input == 0 || input % 2 == 0) return false;
    if (input < 9) return true;
    int max = floor(sqrt((double) input));
    int i = 3;
    while (i < max)
    {
        if (input % i == 0) return false;
        i += 2;
    }
    return true;
}

// ================================================================================
// Public properties
// ================================================================================
long NumberUtil_NextPowerOf_2(int input)
{
    int pow_number = 1;
    return _Next_PowerOf2(input, pow_number);
}

long NumberUtil_NextPrime(int input)
{
    if (_IsPrime(input)) return input;
    return NumberUtil_NextPrime(input + 1);
}