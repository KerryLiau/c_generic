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
        char next = val[i];
        if (next >= '0' && next <= '9') 
        {
            is_num = true;
        } 
        else if (next == '.') 
        {
            dot++;
            // the dot can't type at tail
            bool is_last, is_duplicate;
            is_last = i == strlen(val) - 1;
            is_duplicate = dot > 1;
            if (is_last || is_duplicate)
            {
                is_num = false;
            }
        }
        else if (next == '-')
        {
            bool is_not_first = i != 0;
            if (is_not_first)
            {
                is_num = false;
            }
        }
        else 
        {
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
        } 
        else 
        {
            return GEN_TYPE_INT;
        }
    } 
    else 
    {
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

void _FindItem(GenericTable *table, char *key)
{
    void *item;
    s_out_f("type: %d", GenericTable_ValueType(table, key));
    switch (GenericTable_ValueType(table, key))
    {
        case GEN_TYPE_STR:
        {
            item = GenericTable_Find_Str(table, key);
            s_out_f("%s", ((char*) item));
            break;
        }
        case GEN_TYPE_INT:
        {
            item = GenericTable_Find_Int(table, key);
            s_out_f("%d", *((int*) item));
            break;
        }
        case GEN_TYPE_DOUBLE:
        {
            item = GenericTable_Find_Double(table, key);
            s_out_f("%f", *((double*) item));
            break;
        }
        default:
            break;
    }
}

#define ACT_NON -1
#define ACT_EXIT 0
#define ACT_SET 1
#define ACT_DEL 2
#define ACT_SHOW 3
#define ACT_INCR 4
#define ACT_HELP 5
#define ACT_GET 6

#define CMD_EXIT "exit"
#define CMD_DEL "del"
#define CMD_SET "set"
#define CMD_SHOW "show"
#define CMD_INCR "incr"
#define CMD_HELP "help"
#define CMD_GET "get"

GenericTable *_cmd_table;
void _Init_action_map() 
{
    _cmd_table = New_GenericTable();
    GenericTable_Add(_cmd_table, CMD_DEL, ACT_DEL);
    GenericTable_Add(_cmd_table, CMD_SET, ACT_SET);
    GenericTable_Add(_cmd_table, CMD_GET, ACT_GET);
    GenericTable_Add(_cmd_table, CMD_SHOW, ACT_SHOW);
    GenericTable_Add(_cmd_table, CMD_EXIT, ACT_EXIT);
    GenericTable_Add(_cmd_table, CMD_INCR, ACT_INCR);
    GenericTable_Add(_cmd_table, CMD_HELP, ACT_HELP);
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
                s_out_f("the type of \"%s\" is not integer", key);
                break;
            }
            *num += atoi(val);
            s_out_f("result: %d", *num);
            GenericTable_Add_Int(table, key, *num);
            break;
        }
        case GEN_TYPE_DOUBLE:
        {
            double *num = GenericTable_Find_Double(table, key);
            if (!num)
            {
                s_out_f("the type of \"%s\" is not double", key);
                break;
            }
            *num += atof(val);
            s_out_f("result: %f", *num);
            GenericTable_Add_Double(table, key, *num);
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

void _help()
{
    s_out("set: set var");
    s_out("get: get var");
    s_out("del: delete var");
    s_out("incr: incr var, if var is number");
    s_out("show: show map body in json format");
    s_out("exit: exit the application");
}

int _Pending_action(char *cmd)
{
    int *act = GenericTable_Find_Int(_cmd_table, cmd);
    if (!act)
    {
        return ACT_NON;
    }
    return *act;
}

int main(int argc, char **argv)
{
    _Init_action_map();
    _PrintTable(_cmd_table);
    char *in_buf = (char*) calloc(100, sizeof(char));
    GenericTable *table = New_GenericTable();
    int action;
    s_out("Welcome, please type command below, or type 'help' for more info.");
    while (true)
    {
        printf(">");
        char *cmd, *key;
        cmd = _ReadInput(in_buf);
        action = _Pending_action(cmd);
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
            case ACT_GET:
                key = _ReadKey(in_buf);
                _FindItem(table, key);
                free(key);
                break;
            case ACT_SHOW:
                _PrintTable(table);
                break;
            case ACT_INCR:
                _IncreaseValue(table, in_buf);
                break;
            case ACT_HELP:
                _help();
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
    Delete_GenericTable(&_cmd_table);
    free(in_buf);
}




