#include "math.h"
#include "number_util.h"

static long INT_MAX = 0xFFFFFFFF;

static long NumberUtil_Next_PO2(int input, int pow_number)
{
    int result = (long) pow(2, pow_number);
    if (result > input)
    {
        return result;
    }
    if (result == INT_MAX + 1)
    {
        // Integer 的最大正整數
        return INT_MAX;
    }
    return NumberUtil_Next_PO2(input, pow_number + 1);
}

long NumberUtil_NextPowerOf_2(int input)
{
    int pow_number = 1;
    return NumberUtil_Next_PO2(input, pow_number);
}