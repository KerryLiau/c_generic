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

GenericTypeEnum _TypeOf(char *val)
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

void _PrintTable(GenericTable *table)
{
    char *str = JsonSerializer_TableToIndentStr(table);
    s_out(str);
    free(str);
}

#define BUF_SIZE 1000
char* _ReadInput(char *in_buf)
{
    fgets(in_buf, BUF_SIZE, stdin);
    in_buf[strcspn(in_buf, "\n")] = '\0';
    return strdup(in_buf);
}

void _PutItem(GenericTable *table, char *key, char *in_buf)
{
    s_out("value:");
    char *val = _ReadInput(in_buf);

    switch (_TypeOf(val)) 
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
            free(val);
            break;;
    }

    free(val);
}

#define ACT_NON -1
#define ACT_EXIT 0
#define ACT_SET 1
#define ACT_DEL 2
#define ACT_SHOW 3
#define ACT_INCR 4

#define CMD_EXIT "exit"
#define CMD_DEL "del"
#define CMD_SET "set"
#define CMD_SHOW "show"
#define CMD_INCR "incr"

int _PendingAction(char *cmd)
{
    int result = ACT_NON;
    if (!strcmp(cmd, CMD_DEL)) 
    {
        result = ACT_DEL;
    } 
    else if (!strcmp(cmd, CMD_SET)) 
    {
        result = ACT_SET;
    } 
    else if (!strcmp(cmd, CMD_SHOW)) 
    {
        result = ACT_SHOW;
    }
    else if (!strcmp(cmd, CMD_EXIT)) 
    {
        result = ACT_EXIT;
    }
    else if (!strcmp(cmd, CMD_INCR))
    {
        return ACT_INCR;
    }

    return result;
}

char* _ReadKey(char *in_buf)
{
    s_out("key name:");
    return _ReadInput(in_buf);
}

void _IncreaseValue(GenericTable *table, char *in_buf)
{
    char *key, *val;
    key = _ReadKey(in_buf);
    s_out("value:");
    val = _ReadInput(in_buf);
    switch (_TypeOf(val))
    {
        case GEN_TYPE_INT:
        {
            int *num = GenericTable_Find_Int(table, key);
            if (!num) 
            {
                s_out_f("key %s value is not integer", key);
                break;
            }
            *num += atoi(val);
            s_out_f("result: %d", *num);
            GenericTable_Add_Int(table, key, *num);
            break;
        }
        default:
            break;
    }
    if (key) 
    {
        free(key);
    }
    if (val) 
    {
        free(val);
    }
}

int main(int argc, char **argv)
{
    char *in_buf = (char*) calloc(100, sizeof(char));
    GenericTable *table = New_GenericTable();
    int action;
    while (true)
    {
        s_out("command:");
        char *cmd, *key;
        cmd = _ReadInput(in_buf);
        action = _PendingAction(cmd);

        switch (action) 
        {
            case ACT_EXIT:
                s_out("bye~");
                break;
            case ACT_DEL:
                key = _ReadKey(in_buf);
                GenericTable_Delete(table, key);
                free(key);
                break;
            case ACT_SET:
                key = _ReadKey(in_buf);
                _PutItem(table, key, in_buf);
                free(key);
                break;
            case ACT_SHOW:
                _PrintTable(table);
                break;
            case ACT_INCR:
                _IncreaseValue(table, in_buf);
                break;
            default:
                break;
        }

        if (cmd) 
        {
            free(cmd);
        }
        
        if (action == ACT_EXIT) 
        {
            break;
        }
    }
    Delete_GenericTable(&table);
    free(in_buf);
}




