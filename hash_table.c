#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "hash_table.h"
#include "string_builder.h"
#include "number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
#define VAL_TYPE_STR 0X0
#define VAL_TYPE_INT 0X1
#define VAL_TYPE_LONG 0X2
#define VAL_TYPE_FLOAT 0X3
#define VAL_TYPE_DOUBLE 0X4
#define VAL_TYPE_MAP 0X5
#define VAL_TYPE_ARRAY 0X6

// 實作雜湊表的私有屬性
typedef union
{
    char *s_val;
    int *i_val;
    long *l_val;
    float *f_val;
    double *d_val;
} HashItem;

typedef struct HashTableItem
{
    char *key;
    // char *value;
    HashItem *value;
    int type;
} HashTableItem;

struct HashTable_Private
{
    // 雜湊映射表的容器大小
    int bucket_size;
    /*
     * 實際存在的映射物件(HT_DELETED_ITEM 不算在內)
     * 會隨著刪除方法而減少
     */
    int item_count;
    /*
     * 成功新增、刪除的計數
     * 不會隨著刪除方法而減少
     */
    int modified_count;
    // 承裝映射物件的容器
    HashTableItem **items;
};

// 作為雜湊運算用的常數
static int HASH_ARG_PRIME_1 = 0X83;
static int HASH_ARG_PRIME_2 = 0X1f;
static int DEFAULT_SIZE = 0X1f;

// 作為判定已刪除的物件
static HashTableItem HT_DELETED_ITEM = {NULL, NULL};

static const char *QUOTE = "\"";
static const char *COLON = ":";
static const char *DELIMITER = ",";
static const char *BEGIN = "{";
static const char *END = "}";

static HashTableItem* New_Str_HashTableItem(const char *key, const char *val) 
{
    HashTableItem* item = (HashTableItem*) malloc(sizeof(HashTableItem));
    item->type = VAL_TYPE_STR;
    item->key = strdup(key);
    HashItem *value = (HashItem*) malloc(sizeof(HashItem));
    value->s_val = strdup(val);
    item->value = value;
    return item;
}

static HashTableItem* New_Int_HashTableItem(const char *key, int val) 
{
    HashTableItem* item = (HashTableItem*) malloc(sizeof(HashTableItem));
    item->type = VAL_TYPE_INT;
    item->key = strdup(key);
    HashItem *value = (HashItem*) malloc(sizeof(HashItem));
    int *intVal = (int*) malloc(sizeof(int));
    *intVal = val;
    value->i_val = intVal;
    item->value = value;
    return item;
}

static void Delete_HashTableItem(HashTableItem* item) 
{
    free(item->key);
    free(item->value);
    free(item);
}

static int Calculate_StringHash(const char *str, const int hash_arg) 
{
    long hash = 0;
    const int str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++)
    {
        hash = str[i] * hash_arg ^ (str_len - 1);
    }
    return (int) hash;
}

static int Get_HashValue(const char *str, const int num_buckets, const int attempt)
{
    const int hash_a = Calculate_StringHash(str, HASH_ARG_PRIME_1);
    const int hash_b = Calculate_StringHash(str, HASH_ARG_PRIME_2);
    int result = hash_a + hash_b;
    result += result % num_buckets;
    result += attempt;
    result %= num_buckets;
    return result;
}

static inline int HashTableItem_IsAbandoned(HashTableItem *item)
{
    return item == &HT_DELETED_ITEM;
}

static inline int HashTableItem_IsValid(HashTableItem *item)
{
    return item != NULL && !HashTableItem_IsAbandoned(item);
}

static HashTable* _New_HashTable(int size)
{
    HashTable* table = calloc(1, sizeof(HashTable));
    HashTable_Private *priv = calloc(1, sizeof(HashTable_Private));
    priv->bucket_size = size;
    priv->item_count = 0;
    priv->modified_count = 0;
    priv->items = calloc((size_t) priv->bucket_size, sizeof(HashTableItem*));
    table->priv = priv;
    return table;
}

static void HashTable_PutItem(HashTable *table, HashTableItem *new_item)
{
    HashTable_Private *priv = table->priv;
    int index = Get_HashValue(new_item->key, priv->bucket_size, 0);
    HashTableItem *item = priv->items[index];
    int i = 1;
    while (item != NULL)
    {
        if (HashTableItem_IsAbandoned(item) || strcmp(new_item->key, item->key) != 0)
        {
            index = Get_HashValue(new_item->key, priv->bucket_size, i);
            item = priv->items[index];
            i++;
            continue;
        }
        Delete_HashTableItem(item);
        priv->items[index] = new_item;
        break;
    }
    priv->items[index] = new_item;
    priv->item_count++;
    priv->modified_count++;
}

static void HashTable_Resize(HashTable *table)
{
    int current_size = table->priv->item_count;
    int new_size = NumberUtil_NextPrime(current_size * 2);
    if (DEFAULT_SIZE >= new_size) new_size = DEFAULT_SIZE;
    HashTable *temp_table = _New_HashTable(new_size);
    if (!temp_table)
    {
        printf("malloc new HashTable failed...\n");
    }
    HashTable_Private *curr_priv = table->priv;
    for (size_t i = 0; i < curr_priv->bucket_size; i++)
    {
        HashTableItem *item = curr_priv->items[i];
        if (HashTableItem_IsValid(item))
        {
            HashTable_PutItem(temp_table, item);
        }
    }
    HashTable_Private *temp_priv = temp_table->priv;
    table->priv = temp_priv;
    free(curr_priv->items);
    if (curr_priv)
    {
        free(curr_priv);
    }
}

static int HashTable_NeedResize(HashTable *table)
{
    HashTable_Private *priv = table->priv;
    return priv->modified_count * 100 / priv->bucket_size > 75;
}

static inline void HashTable_EnsureTableSize(HashTable *table)
{
    if (HashTable_NeedResize(table)) {
        HashTable_Resize(table);
    }
}

static HashTableItem* HashTable_Find(HashTable *table, const char *key)
{
    HashTable_Private *priv = table->priv;
    int index = Get_HashValue(key, priv->bucket_size, 0);
    HashTableItem *item = priv->items[index];
    int i = 1;
    while (item != NULL)
    {
        if (HashTableItem_IsAbandoned(item) || strcmp(item->key, key) != 0)
        {
            index = Get_HashValue(key, priv->bucket_size, i);
            item = priv->items[index];
            i++;
            continue;
        }
        return item;
    }
    return NULL;
}

// ================================================================================
// Public properties
// ================================================================================
HashTable* New_HashTable() 
{
    return _New_HashTable(DEFAULT_SIZE);
}

void Delete_HashTable(HashTable **p_to_table) 
{
    HashTable *table = *p_to_table;
    HashTable_Private *priv = table->priv;
    for (int i = 0; i < priv->bucket_size; i++) 
    {
        HashTableItem *item = priv->items[i];
        if (HashTableItem_IsValid(item)) Delete_HashTableItem(item);
    }
    free(priv->items);
    free(priv);
    free(table);
    *p_to_table = NULL;
} 

void HashTable_Put_Str(HashTable *table, const char *key, const char *value)
{
    HashTable_EnsureTableSize(table);
    HashTableItem *new_item = New_Str_HashTableItem(key, value);
    HashTable_PutItem(table, new_item);
}



void HashTable_Put_Int(HashTable *table, const char *key, int value)
{
    HashTable_EnsureTableSize(table);
    HashTableItem *new_item = New_Int_HashTableItem(key, value);
    HashTable_PutItem(table, new_item);
}

char* HashTable_Find_Str(HashTable *table, const char *key)
{
    HashTableItem *item = HashTable_Find(table, key);
    if (!item || item->type != VAL_TYPE_STR) return NULL;
    return item->value->s_val;
}

int* HashTable_Find_Int(HashTable *table, const char *key)
{
    HashTableItem *item = HashTable_Find(table, key);
    if (!item || item->type != VAL_TYPE_INT) return NULL;
    int *result = (item->value->i_val);
    return result;
}

void HashTable_Delete(HashTable *table, const char *key)
{
    HashTable_Private *priv = table->priv;
    int index = Get_HashValue(key, priv->bucket_size, 0);
    HashTableItem *item = priv->items[index];
    int i = 1;
    while (item != NULL)
    {
        if (HashTableItem_IsAbandoned(item) || strcmp(item->key, key) != 0)
        {
            index = Get_HashValue(key, priv->bucket_size, i);
            item = priv->items[index];
            i++;
            continue;
        }
        Delete_HashTableItem(item);
        priv->items[index] = &HT_DELETED_ITEM;
        priv->item_count--;
        priv->modified_count++;
        break;
    }
    HashTable_EnsureTableSize(table);
}

inline int HashTable_Size(HashTable *table)
{
    return table->priv->item_count;
}

inline int HashTable_IsEmpty(HashTable *table)
{
    return HashTable_Size(table) == 0;
}

char* HashTable_ToString(HashTable *table)
{
    int counter = 0;
    StringBuilder *builder = New_StringBuilder();
    StringBuilder_Append(builder, BEGIN);
    HashTable_Private *priv = table->priv;

    for (size_t i = 0; i < priv->bucket_size; i++)
    {
        HashTableItem *item = priv->items[i];
        if (!HashTableItem_IsValid(item)) continue;

        if (counter > 0) StringBuilder_Append(builder, DELIMITER);
        StringBuilder_Append(builder, QUOTE);
        StringBuilder_Append(builder, item->key);
        StringBuilder_Append(builder, QUOTE);
        StringBuilder_Append(builder, COLON);
        
        int is_str = item->type == VAL_TYPE_STR;
        if (is_str) StringBuilder_Append(builder, QUOTE);
        
        switch (item->type) 
        {
            case VAL_TYPE_STR:
                StringBuilder_Append(builder, item->value->s_val);
                break;
            case VAL_TYPE_INT:
                StringBuilder_Append(builder, *(item->value->i_val));
                break;
            case VAL_TYPE_FLOAT:
                StringBuilder_Append(builder, *(item->value->f_val));
                break;
            case VAL_TYPE_DOUBLE:
                StringBuilder_Append(builder, *(item->value->d_val));
                break;
        }
        
        if (is_str) StringBuilder_Append(builder, QUOTE);
        counter++;
    }
    
    StringBuilder_Append(builder, END);
    char *str = StringBuilder_Value(builder);
    Delete_StringBuilder(&builder);
    return str;
}

