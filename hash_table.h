#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct HashTable_Private HashTable_Private;

typedef struct 
{
    HashTable_Private *priv;
} HashTable;

HashTable* New_HashTable();

void Delete_HashTable(HashTable **table);

#define HashTable_Put(table, k, v) _Generic((v),\
    const char*: HashTable_Put_Str,\
    char*: HashTable_Put_Str,\
    int: HashTable_Put_Int\
)(table, k, v)
/*
    long: HashTable_Put_Long,\
    double: HashTable_Put_Double,\
    float: HashTable_Put_Float\
*/
void HashTable_Put_Str(HashTable *table, const char *key, const char *value);

void HashTable_Put_Int(HashTable *table, const char *key, int i);

char* HashTable_Find_Str(HashTable *table, const char *key);

int* HashTable_Find_Int(HashTable *table, const char *key);

void HashTable_Delete(HashTable *table, const char *key);

char* HashTable_ToString(HashTable *table);

int HashTable_Size(HashTable *table);

int HashTable_IsEmpty(HashTable *table);

#endif