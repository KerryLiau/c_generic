#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hash_table.h"
#include "string_builder.h"
#include "common_util.h"

void HashTable_Simple_Test(void)
{
    s_out("\n\nBegin test HashTable");
    HashTable* table = New_HashTable();
    s_out_f("the hash table address: '%p'", table);
    
    s_out("\nlet's try add key value in map");
    const char *key = "Hello";
    const char *value = "World";
    HashTable_Add(table, key, value);

    char *v = HashTable_Find_Str(table, key);
    s_out_f("the key is '%s', and the value is '%s'", key, v);
    int count = HashTable_Size(table);
    s_out_f("now the map size is: %d", count);

    char *table_str = HashTable_ToJsonStr(table);
    s_out_f("the hash map now looks like: %s", table_str);

    s_out_f("\nlet's try update value stored in key %s", key);
    const char *new_value = "I'm Kerry";
    HashTable_Add(table, key, new_value);
    v = HashTable_Find_Str(table, key);
    if (strcmp(v, new_value) == count)
    {
        s_out_f("now the value stored in '%s' has change to '%s'", key, new_value);
    }
    if (HashTable_Size(table) == 1)
    {
        s_out("and the map count is still the same");
    }

    if (strcmp(value, v) == 0)
    {
        s_out("the value from hash table is the same as input");
    }

    table_str = HashTable_ToJsonStr(table);
    s_out_f("the hash map now looks like: %s", table_str);
    free(table_str);

    s_out_f("\nlet's try remove key %s", key);
    HashTable_Delete(table, key);
    s_out_f("the map size is: %d", HashTable_Size(table));
    v = HashTable_Find_Str(table, key);
    if (v == NULL)
    {
        s_out_f("the key %s is now removed", key);
    }
    
    s_out_f("\nlet's try destruct table %p", table);
    Delete_HashTable(&table);
    if (table == NULL)
    {
        s_out_f("the table was delete successfully, now the address is '%p'", table);
    }
    else 
    {
        s_out_f("the table was failed to delete, now the address is: '%p'", table);
    }
}

void HashTable_Resize_Test()
{
    s_out("\n\nNow, let's try put and delete more intensively");
    HashTable *table = New_HashTable_WithBucketSize(200);
    StringBuilder *key_builder = New_StringBuilder();
    StringBuilder *value_builder = New_StringBuilder();

    int count = 100;
    s_out_f("First, let's put %d element in table", count);
    for (size_t i = 0; i < count; i++)
    {
        StringBuilder_Append(key_builder, (int) i);
        StringBuilder_Append(value_builder, (int) i);
        HashTable_Add_Str(
            table, StringBuilder_Value(key_builder), StringBuilder_Value(value_builder)
        );
        StringBuilder_Clear(key_builder);
        StringBuilder_Clear(value_builder);
    }

    s_out_f("finished, now the table size is %d", HashTable_Size(table));
    char *table_str1 = HashTable_ToJsonStr(table);
    s_out_f("now the table looks like below:\n%s", table_str1);
    free(table_str1);

    s_out("Second, let's delete all element in table");
    for (size_t i = 0; i < count; i++)
    {
        StringBuilder_Append(key_builder, (int) i);
        HashTable_Delete(
            table, StringBuilder_Value(key_builder)
        );
        StringBuilder_Clear(key_builder);
    }

    char *table_str;
    if (HashTable_IsEmpty(table))
    {
        table_str = HashTable_ToJsonStr(table);
        s_out_f("now the table is empty, and looks like this: %s", table_str);
        free(table_str);
    }

    Delete_HashTable(&table);
    Delete_StringBuilder(&key_builder);
    Delete_StringBuilder(&value_builder);
}

void Time_Test(void)
{
    time_t begin, end;
    begin = clock();

    HashTable_Simple_Test();
    HashTable_Resize_Test();
    end = clock();

    double period = end - begin;
    s_out_f("elapsed milli seconds: %f", period / CLOCKS_PER_SEC * 1000);
}

void Generic_Test(void)
{
    s_out("\n\nBegin HashTable generic value test");

    HashTable *table = New_HashTable();
    HashTable_Add(table, "intVal", 26);
    HashTable_Add(table, "strVal", "foo");

    s_out_f("strVal: %s", HashTable_Find_Str(table, "strVal"));
    s_out_f("intVal: %d", *HashTable_Find_Int(table, "intVal"));

    int *ss = HashTable_Find_Int(table, "ss");
    if (!ss) 
    {
        s_out("ss is NULL");
    }
    else 
    {
        s_out_f("intVal: %d", *ss);
    }
    s_out_f("%s", HashTable_ToIndentJsonStr(table));

    Delete_HashTable(&table);
}

void Dynamic_Type(void)
{
    s_out("\n\nBegin HashTable dynamic value type test");

    HashTable *table = New_HashTable();
    HashTable_Add(table, "val", 26);
    s_out_f("%d", *HashTable_Find_Int(table, "val"));

    HashTable_Add(table, "val", "AAA");
    s_out_f("%s", HashTable_Find_Str(table, "val"));

    HashTable_Add(table, "val", 5.56);
    s_out_f("%f", *HashTable_Find_Double(table, "val"));

    s_out_f("%s", HashTable_ToIndentJsonStr(table));
    Delete_HashTable(&table);
}

int main(void)
{
    Time_Test();
    Generic_Test();
    Dynamic_Type();
}
