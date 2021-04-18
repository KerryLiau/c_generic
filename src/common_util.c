#include "stdio.h"

#include "../include/common_util.h"
#include "../include/common_util.h"

void println_str(char *var)
{
    printf("%s\n", var);
}

void println_int(int var)
{
    printf("%d\n", var);
}

void println_long(long var)
{
    printf("%ld\n", var);
}

void println_double(double var)
{
    printf("%f\n", var);
}

void println_float(float var)
{
    printf("%f\n", var);
}

void printerrln_str(char *var)
{
    printf("!Warning: %s\n", var);
}

bool CommonUtil_IsNull(void *obj)
{
    if (!obj)
    {
        s_out_err("null object deteced!");
        return true;
    }
    return false;
}