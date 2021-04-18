#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../include/common_util.h"
#include "../include/generic_table.h"
#include "../include/generic_type.h"
#include "../include/string_builder.h"
#include "../include/number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
struct GenericTableItem
{
    char *key;
    GenericType *value;
};

struct GenericTable_Private
{
    /**
     * 雜湊映射表的容器大小
     */
    int bucket_size;
    /**
     * 重構臨界點，用以判定容器是否擴充、縮減
     */
    int resize_threshold;
    /**
     * 實際存在的映射物件(HT_DELETED_ITEM 不算在內)
     * 會隨著刪除方法而減少
     */
    int item_count;
    /**
     * 成功新增、刪除的計數
     * 不會隨著刪除方法而減少
     */
    int modified_count;
    /**
     * 承裝映射物件的容器
     */
    GenericTableItem **items;
};

/**
 * 迭代器實作
 */
struct GenericTableIterator 
{
    int last;
    int next;
    GenericTableItem **items;
};

// 作為雜湊運算用的常數
static const int _HASH_ARG = 0X11;
static const int _DEFAULT_SIZE = 0X1f;
static const int _DEFAULT_LOAD_FACTOR = 0X50;

// 作為判定已刪除的物件
static GenericTableItem _DELETED_ITEM = {NULL, NULL};

static GenericTableItem* _New_GenericTableItem(const char *key, GenericType *val)
{
    GenericTableItem* item = (GenericTableItem*) malloc(sizeof(GenericTableItem));
    item->key = strdup(key);
    item->value = val;

    return item;
}

static void _Delete_GenericTableItem(GenericTableItem* item) 
{
    free(item->key);
    Delete_GenericType(&(item->value));
    free(item);
}

static int _Calculate_StringHash(const char *str, const int hash_arg) 
{
    long hash = 0;
    const int str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++)
    {
        hash <<= 6;
        hash += str[i] * hash_arg ^ (str_len - 1);
        hash ^= (hash >> 16) ^ (hash >> 8);
    }
    hash ^= (hash >> 24) ^ (hash >> 12);
    return (int) hash ^ (hash >> 8) ^ (hash >> 4);
}

static int _Get_HashValue(const char *str, const int max_val, const int addition)
{
    int result = _Calculate_StringHash(str, _HASH_ARG);
    if (result < 0) result = ~result;

    return (result + addition) % max_val;
}

static inline int _IsAbandoned(GenericTableItem *item)
{
    return item == &_DELETED_ITEM;
}

static inline int _IsValid(GenericTableItem *item)
{
    return item != NULL && !_IsAbandoned(item);
}

static GenericTable* _New_GenericTable(int size, int threshold)
{
    GenericTable* table = calloc(1, sizeof(GenericTable));
    GenericTable_Private *priv = calloc(1, sizeof(GenericTable_Private));

    priv->bucket_size = size;
    priv->resize_threshold = threshold;
    priv->item_count = 0;
    priv->modified_count = 0;
    priv->items = calloc((size_t) priv->bucket_size, sizeof(GenericTableItem*));
    table->priv = priv;

    return table;
}

static void _AddItem(GenericTable *table, GenericTableItem *new_item)
{
    GenericTable_Private *priv = table->priv;
    GenericTableItem *item;
    int index, addition;
    
    addition = 0;
    while (true)
    {
        index = _Get_HashValue(new_item->key, priv->bucket_size, addition);
        item = priv->items[index];
        addition++;

        if (item == NULL) break;
        if (_IsAbandoned(item) || strcmp(new_item->key, item->key) != 0) continue;

        _Delete_GenericTableItem(item);
        priv->item_count--;
        break;
    }

    priv->items[index] = new_item;
    priv->item_count++;
    priv->modified_count++;
}

static int _NeedResize(GenericTable *table)
{
    GenericTable_Private *priv = table->priv;
    return priv->modified_count * 100 / priv->bucket_size > priv->resize_threshold;
}

static inline void _EnsureBucketSize(GenericTable *table)
{
    if (!_NeedResize(table)) return;
    
    GenericTable_Private *curr_priv = table->priv;
    int current_size = curr_priv->item_count;
    int new_size = NumberUtil_NextPrime(current_size * 2);
    if (_DEFAULT_SIZE >= new_size) 
    {
        new_size = _DEFAULT_SIZE;
    }

    GenericTable *temp_table = _New_GenericTable(new_size, curr_priv->resize_threshold);
    if (!temp_table) s_out_err("malloc new GenericTable failed");

    for (size_t i = 0; i < curr_priv->bucket_size; i++)
    {
        GenericTableItem *item = curr_priv->items[i];
        if (!_IsValid(item)) continue;

        _AddItem(temp_table, item);
    }

    GenericTable_Private *temp_priv = temp_table->priv;
    table->priv = temp_priv;
    free(curr_priv->items);

    if (curr_priv) return;
    free(curr_priv);
}

static GenericTableItem* _Find(GenericTable *table, const char *key)
{
    GenericTable_Private *priv = table->priv;
    GenericTableItem *item;
    int index, addition;

    addition = 0;
    while (true)
    {
        index = _Get_HashValue(key, priv->bucket_size, addition);
        item = priv->items[index];
        addition++;

        if (item == NULL) break;
        if (_IsAbandoned(item) || strcmp(item->key, key) != 0) continue;

        return item;
    }

    return NULL;
}

// ================================================================================
// Public properties
// ================================================================================
GenericTable* New_GenericTable(void) 
{
    return _New_GenericTable(_DEFAULT_SIZE, _DEFAULT_LOAD_FACTOR);
}

GenericTable* New_GenericTable_WithBucketSize(int size) 
{
    return _New_GenericTable(size, _DEFAULT_LOAD_FACTOR);
}

GenericTable* New_GenericTable_WithBucketSizeAndLoadFactor(int size, int load_factor)
{
    return _New_GenericTable(size, load_factor);
}

void Delete_GenericTable(GenericTable **p_to_table) 
{
    GenericTable *table = *p_to_table;
    GenericTable_Private *priv = table->priv;
    for (int i = 0; i < priv->bucket_size; i++) 
    {
        GenericTableItem *item = priv->items[i];
        if (!_IsValid(item)) continue;

        _Delete_GenericTableItem(item);
    }

    free(priv->items);
    free(priv);
    free(table);
    *p_to_table = NULL;
} 

void GenericTable_Add_Str(GenericTable *table, const char *key, const char *value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

void GenericTable_Add_Int(GenericTable *table, const char *key, int value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

void GenericTable_Add_Long(GenericTable *table, const char *key, long value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

void GenericTable_Add_Double(GenericTable *table, const char *key, double value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

void GenericTable_Add_Float(GenericTable *table, const char *key, float value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

void GenericTable_Add_Table(GenericTable *table, const char *key, GenericTable *value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

void GenericTable_Add_List(GenericTable *table, const char *key, struct GenericList *value)
{
    _EnsureBucketSize(table);
    GenericType *gen_obj = New_GenericType(value);
    GenericTableItem *new_item = _New_GenericTableItem(key, gen_obj);
    _AddItem(table, new_item);
}

char* GenericTable_Find_Str(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    if (!item) return NULL;

    GenericType *gen = item->value;
    if (!item || !GenericType_IsType(gen, GEN_TYPE_STR)) return NULL;
    
    return GenericType_GetStr(gen);
}

int* GenericTable_Find_Int(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    if (!item) return NULL;

    GenericType *gen = item->value;
    if (!item || !GenericType_IsType(gen, GEN_TYPE_INT)) return NULL;
    
    return GenericType_GetInt(gen);
}

long* GenericTable_Find_Long(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    if (!item) return NULL;

    GenericType *gen = item->value;
    if (!item || !GenericType_IsType(gen, GEN_TYPE_LONG)) return NULL;
    
    return GenericType_GetLong(gen);
}

double* GenericTable_Find_Double(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    if (!item) return NULL;

    GenericType *gen = item->value;
    if (!item || !GenericType_IsType(gen, GEN_TYPE_DOUBLE)) return NULL;
    
    return GenericType_GetDouble(gen);
}

float* GenericTable_Find_Float(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    if (!item) return NULL;

    GenericType *gen = item->value;
    if (!item || !GenericType_IsType(gen, GEN_TYPE_FLOAT)) return NULL;
    
    return GenericType_GetFloat(gen);
}

GenericTable* GenericTable_Find_Table(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    if (!item) return NULL;

    GenericType *gen = item->value;
    if (!item || !GenericType_IsType(gen, GEN_TYPE_TABLE)) return NULL;
    
    return GenericType_GetTable(gen);
}

void GenericTable_Delete(GenericTable *table, const char *key)
{
    GenericTable_Private *priv = table->priv;
    GenericTableItem *item;
    int index, addition;

    addition = 0;
    while (true)
    {
        index = _Get_HashValue(key, priv->bucket_size, addition);
        item = priv->items[index];
        addition++;

        if (item == NULL) break;
        if (_IsAbandoned(item) || strcmp(item->key, key) != 0) continue;

        _Delete_GenericTableItem(item);
        priv->items[index] = &_DELETED_ITEM;
        priv->item_count--;
        priv->modified_count++;
        break;
    }

    _EnsureBucketSize(table);
}

bool GenericTable_HasKey(GenericTable *table, const char *key)
{
    GenericTableItem *item = _Find(table, key);
    return _IsValid(item);
}

int GenericTable_Size(GenericTable *table)
{
    return table->priv->item_count;
}

inline bool GenericTable_IsEmpty(GenericTable *table)
{
    return GenericTable_Size(table) == 0;
}

char* GenericTableItem_GetKey(GenericTableItem *item)
{
    return item->key;
}

struct GenericType* GenericTableItem_GetValue(GenericTableItem *item)
{
    return item->value;
}

GenericTableIterator* GenericTable_GetIterator(GenericTable *table)
{
    int iterator_size = table->priv->item_count;
    GenericTableIterator *iterator = (GenericTableIterator*) malloc(sizeof(GenericTableIterator));
    GenericTableItem **items = (GenericTableItem**) calloc(iterator_size, sizeof(GenericTableItem*));
    iterator->next = 0;
    iterator->last = 0;
    iterator->items = items;
    for (int i = 0; i < table->priv->bucket_size; i++) 
    {
        GenericTableItem *item = table->priv->items[i];
        if (!_IsValid(item)) continue;
        iterator->items[iterator->last] = item;
        iterator->last++;
    }
    return iterator;
}

bool GenericTableIterator_HasNext(GenericTableIterator *iterator)
{
    return iterator->next < iterator->last;
}

GenericTableItem* GenericTableIterator_Next(GenericTableIterator *iterator)
{
    GenericTableItem *item = iterator->items[iterator->next];
    iterator->next++;
    return item;
}

void Delete_GenericTableIterator(GenericTableIterator **p_iterator)
{
    GenericTableIterator *iterator = *p_iterator;
    // iterator->items 中的 GenericTableItem 還需要使用，
    // 故釋放 iterator->items 本身，以及歸零裡面的指標即可
    for (int i = 0; i < iterator->last; i++)
    {
        GenericTableItem **p = &(iterator->items[i]);
        *p = NULL;
    }
    free(iterator->items);
    free(iterator);
    *p_iterator = NULL;
}





