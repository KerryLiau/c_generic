#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/json_serializer.h"
#include "../../include/generic_list.h"
#include "../../include/generic_table.h"
#include "../../include/generic_type.h"
#include "../../include/string_builder.h"
#include "../../include/common_util.h"

void GenericTable_Simple_Test(void)
{
    s_out("\n\nBegin basic test GenericTable");
    GenericTable* table = New_GenericTable();
    s_out_f("the generic table address: '%p'", table);
    
    s_out("\nlet's try add key value in table");
    const char *key = "Hello";
    const char *value = "World";
    GenericTable_Add(table, key, value);

    char *v = GenericTable_Find_Str(table, key);
    if (strcmp(value, v) == 0)
    {
        s_out("the value from generic table is the same as input");
    }
    s_out_f("the key is '%s', and the value is '%s'", key, v);
    int count = GenericTable_Size(table);
    s_out_f("now the table size is: %d", count);

    char *table_str = JsonSerializer_ToStr(table);
    s_out_f("the generic table now looks like: %s", table_str);

    s_out_f("\nlet's try update value stored in key %s", key);
    const char *new_value = "I'm Kerry";
    GenericTable_Add(table, key, new_value);
    v = GenericTable_Find_Str(table, key);
    if (strcmp(v, new_value) == 0)
    {
        s_out_f("now the value stored in '%s' has change to '%s'", key, new_value);
    }
    if (GenericTable_Size(table) == 1)
    {
        s_out("and the table count is still the same");
    }

    table_str = JsonSerializer_ToStr(table);
    s_out_f("the generic table now looks like: %s", table_str);
    free(table_str);

    s_out_f("\nlet's try remove key %s", key);
    GenericTable_Delete(table, key);
    v = GenericTable_Find_Str(table, key);
    if (v == NULL)
    {
        s_out_f("the key %s is now removed", key);
    }
    if (GenericTable_IsEmpty(table))
    {
        s_out("the table is now empty");
    }

    s_out("\nlet's test function 'GenericTable_HasKey'");
    s_out("first check key 'foo' that currently not exist in table");
    if (!GenericTable_HasKey(table, "foo"))
    {
        s_out("OK, the result is: key 'foo' not in table yet");
    }
    s_out("next put 'foo' into table, link it with value 'bar', and check again");
    GenericTable_Add(table, "foo", "bar");
    if (GenericTable_HasKey(table, "foo"))
    {
        s_out_f("the result is: key 'foo' is in table, the value is %s", GenericTable_Find_Str(table, "foo"));
    }
    
    s_out_f("\nlet's try destruct table %p", table);
    Delete_GenericTable(&table);
    if (table == NULL)
    {
        s_out_f("the table was delete successfully, now the address is '%p'", table);
    }
    else 
    {
        s_out_f("the table was failed to delete, now the address is: '%p'", table);
    }
}

void GenericTable_Resize_Test()
{
    s_out("\n\nNow, let's try put and delete more intensively\n");
    StringBuilder *key_builder = New_StringBuilder();

    int count = 100 * 100 * 100;
    s_out_f(
        "First, let's put %d element in table, key is from 1999999999 to %d", count, 1999999999 + (count - 1)
    );
    s_out("to increase calculate complexity, we add prefix and suffix");
    s_out("'Prefix_' as static prefix, and '_Suffix' as static key suffix\n");
    s_out("if you're going to put very large amount into generic table, for better performance, you may set table init bucket size");
    GenericTable *table = New_GenericTable_WithBucketSize(count * 2);
    
    for (int i = 0; i < count; i++)
    {
        StringBuilder_Append(key_builder, "Prefix_");
        StringBuilder_Append(key_builder, i + 1999999999);
        StringBuilder_Append(key_builder, "_Suffix");
        GenericTable_Add(table, StringBuilder_Value(key_builder), (int) i);
        StringBuilder_Clear(key_builder);
    }

    s_out_f("finished, now the table size is %d, but the table is too large to print, let's just skip that", GenericTable_Size(table));
    s_out("Second, let's delete all element in table\n");
    for (int i = 0; i < count; i++)
    {
        StringBuilder_Append(key_builder, "Prefix_");
        StringBuilder_Append(key_builder, i + 1999999999);
        StringBuilder_Append(key_builder, "_Suffix");
        GenericTable_Delete(table, StringBuilder_Value(key_builder));
        StringBuilder_Clear(key_builder);
    }

    char *table_str;
    if (GenericTable_IsEmpty(table))
    {
        table_str = JsonSerializer_ToStr(table);
        s_out_f("now the table is empty, and looks like this: %s", table_str);
        free(table_str);
    }

    Delete_GenericTable(&table);
    Delete_StringBuilder(&key_builder);
}

void Time_Test(void)
{
    time_t begin, end;

    begin = clock();
    GenericTable_Simple_Test();
    GenericTable_Resize_Test();
    end = clock();

    double period = end - begin;
    s_out_f("elapsed milli seconds: %f", period / CLOCKS_PER_SEC * 1000);
}

void Generic_Test(void)
{
    s_out("\n\nBegin GenericTable generic value test");

    GenericTable *table = New_GenericTable();
    GenericTable_Add(table, "intVal", 26);
    GenericTable_Add(table, "strVal", "foo");
    GenericTable_Add(table, "const char *str", "*str");

    s_out_f("strVal: %s", GenericTable_Find_Str(table, "strVal"));
    s_out_f("intVal: %d", *GenericTable_Find_Int(table, "intVal"));

    int *ss = GenericTable_Find_Int(table, "ss");
    if (!ss) 
    {
        s_out("ss is NULL");
    }
    else 
    {
        s_out_f("intVal: %d", *ss);
    }

    char *json_str = JsonSerializer_ToIndentStr(table);
    s_out_f("%s", json_str);
    free(json_str);
    Delete_GenericTable(&table);
}

void Dynamic_Type(void)
{
    s_out("\n\nBegin GenericTable dynamic value type test\n");

    GenericTable *table = New_GenericTable();
    s_out("table key 'val' with 26");
    GenericTable_Add(table, "val", 26);
    s_out_f("the key 'val' correspond value is: %d\n", *GenericTable_Find_Int(table, "val"));

    s_out("table key 'val' with 'AAA'");
    GenericTable_Add(table, "val", "AAA");
    s_out_f("the key 'val' correspond value is: %s\n", GenericTable_Find_Str(table, "val"));

    s_out("table key 'val' with 5.56");
    GenericTable_Add(table, "val", 5.56);
    s_out_f("the key 'val' correspond value is: %f\n", *GenericTable_Find_Double(table, "val"));

    char *json_str = JsonSerializer_ToIndentStr(table);
    s_out_f("%s", json_str);
    free(json_str);
    Delete_GenericTable(&table);
}

void NestHybridStructure_Test()
{
    s_out("\n\nBegin test nest and hybrid structure\n");

    s_out("let's create table 1,2,3");
    GenericTable *table1 = New_GenericTable();
    GenericTable *table2 = New_GenericTable();
    GenericTable *table3 = New_GenericTable();

    s_out("and put table 2 into table 1, then table 3 into table 2");
    GenericTable_Add(table1, "Table 2", table2);
    GenericTable_Add(table2, "Table 3", table3);

    s_out("put some other element into table 1,2,3");
    GenericTable_Add(table1, "My name is", "Table 1");
    GenericTable_Add(table2, "My name is", "Table 2");
    GenericTable_Add(table3, "My name is", "Table 3");

    GenericList *list1 = New_GenericList();
    GenericList_Add(list1, 5);
    GenericList_Add(list1, "6");
    GenericList_Add(list1, 7.62);

    GenericTable_Add(table1, "List 1", list1);

    s_out("OK, let's see how table 1 looks like now:\n");
    char *str = JsonSerializer_ToIndentStr(table1);
    s_out(str);
    free(str);

    s_out("\ntime for delete table, because 'Delete_GenericTable' function will delete table recursive, so we only need delete table 1");
    Delete_GenericTable(&table1);
}

int main(int argc, char** argv)
{
    Time_Test();
    Generic_Test();
    Dynamic_Type();
    NestHybridStructure_Test();
}







