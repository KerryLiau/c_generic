#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include/json_serializer.h"
#include "include/generic_list.h"
#include "include/generic_table.h"
#include "include/generic_type.h"
#include "include/string_builder.h"
#include "include/common_util.h"

// static const int size = 100;
#define size 100

int main(int argc, char **argv)
{
    char *string = (char*) calloc(100, sizeof(char));
    while (true)
    {
        s_out("please input:");
        fgets(string, size, stdin);
        if (strcmp(string, "exit") == 0) //strcmp(string, "掰掰")
        {
            s_out("掰掰～");
            break;
        }
        else 
        {
            s_out(strcmp(string, "掰掰"));
        }
        s_out(string);
    }
}




