#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include/generic_table.h"
#include "include/generic_table.h"
#include "include/generic_type_enum.h"
#include "include/json_serializer.h"
#include "include/generic_list.h"
#include "include/generic_table.h"
#include "include/generic_type.h"
#include "include/string_builder.h"
#include "include/common_util.h"

GenericTypeEnum type_of(char *val)
{
    GenericTypeEnum result;
    bool is_num = true;
    int dot = 0;
    for (int i = 0; i < strlen(val); i++)
    {
        if (val[i] >= '0' && val[i] <= '9') 
        {
            is_num = true;
        } else if (val[i] == '.') {
            dot++;
            if (i == strlen(val) - 1 || i == 0 || dot > 1) 
            {
                is_num = false;
            }
        } else {
            is_num = false;
        }
        if (!is_num)
        {
            break;
        }
    }
    if (is_num) 
    {
        if (dot)
        {
            return GEN_TYPE_DOUBLE;
        } else {
            return GEN_TYPE_INT;
        }
    } else {
        return GEN_TYPE_STR;
    }
}

#define size 1000
#define Put 1
#define Delete 2
int main(int argc, char **argv)
{
    char *string = (char*) calloc(100, sizeof(char));
    GenericTable *table = New_GenericTable();
    int operate = 0;
    while (true)
    {
        s_out("command:");
        fgets(string, size, stdin);
        string[strcspn(string, "\n")] = '\0';
        char *cmd = strdup(string);
        if (!strcmp(cmd, "rm")) {
            operate = Delete;
        } 
        else if (!strcmp(cmd, "add")) 
        {
            operate = Put;
        }

        if (strcmp(string, "exit") == 0) 
        {
            free(cmd);
            s_out("bye~");
            break;
        }

        s_out("key name:");
        fgets(string, size, stdin);
        string[strcspn(string, "\n")] = '\0';
        char *key = strdup(string);

        if (operate == Delete)
        {
            GenericTable_Delete(table, key);
        }
        else 
        {
            s_out("value:");
            fgets(string, size, stdin);
            string[strcspn(string, "\n")] = '\0';
            char *val = strdup(string);

            switch (type_of(val)) 
            {
                case GEN_TYPE_STR:
                    GenericTable_Add(table, key, val);
                    break;
                case GEN_TYPE_INT:
                    GenericTable_Add(table, key, atoi(val));
                    break;
                case GEN_TYPE_DOUBLE:
                    GenericTable_Add(table, key, atof(val));
                    break;
                // 以下型別先不進行處裡
                case GEN_TYPE_LONG:
                case GEN_TYPE_FLOAT:
                case GEN_TYPE_TABLE:
                case GEN_TYPE_LIST:
                    break;
                default:
                    return false;
            }
            free(val);
        }

        char *str = JsonSerializer_TableToIndentStr(table);
        s_out(str);
        free(str);
        free(key);
        free(cmd);
        if (strcmp(string, "exit") == 0)
        {
            s_out("bye~");
            break;
        }
    }
    Delete_GenericTable(&table);
    free(string);
}




