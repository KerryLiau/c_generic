#ifndef BOOLEAN_H
#define BOOLEAN_H

#include "stdio.h"

/**
 * 增進可讀性用，並非型別安全
 */
#define bool int
#define true 1
#define false 0
#define out printf

#define printfln(formate, ...) \
    {\
        char* newFormate = (char*) malloc(sizeof(char) * (strlen(formate) + 4));\
        strcpy(newFormate, formate);\
        const char* newLine = "\n";\
        strcat(newFormate, newLine);\
        printf(newFormate, ##__VA_ARGS__);\
        free(newFormate);\
    }
#define s_out_f printfln

#define printErrfln(formate, ...) \
    {\
        const char *warn = "!Warning: ";\
        char* newFormate = (char*) malloc(sizeof(char) * (strlen(formate) + strlen(warn) + 4));\
        strcpy(newFormate, formate);\
        const char* newLine = "\n";\
        strcat(newFormate, newLine);\
        printf(newFormate, ##__VA_ARGS__);\
        free(newFormate);\
    }
#define s_out_err_f printErrfln

#define s_out(var) _Generic((var),\
    char*: println_str,\
    int: println_int,\
    long: println_long,\
    double: println_double,\
    float: println_float\
    ) (var)

void println_str(char *var);

void println_int(int var);

void println_long(long var);

void println_double(double var);

void println_float(float var);

void printerrln_str(char *var);
#define s_out_err printerrln_str

bool CommonUtil_IsNull(void *obj);

#endif