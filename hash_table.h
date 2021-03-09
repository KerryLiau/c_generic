#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct HashTable_Private HashTable_Private;

typedef struct 
{
    HashTable_Private *priv;
} HashTable;

HashTable* New_HashTable();

void Delete_HashTable(HashTable **table);

void HashTable_Put(HashTable *table, const char *key, const char *value);

char* HashTable_Find(HashTable *table, const char *key);

void HashTable_Delete(HashTable *table, const char *key);

char* HashTable_ToString(HashTable *table);

int HashTable_Size(HashTable *table);

int HashTable_IsEmpty(HashTable *table);

#endif