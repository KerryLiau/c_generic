#include "math.h"
#include "number_util.h"

static long LONG_MAX = 0xFFFFFFFFFFFFFFFF / 2;

long NumberUtil_NextPowerOf_2(int input)
{
    long result = 2;
    int pow_n = 1;
    while (1)
    {
        result = (long) pow(2, pow_n);
        if (result > input)
        {
            return result;
        }
        if (result == LONG_MAX)
        {
            return LONG_MAX;
        }
        pow_n++;
    }
}