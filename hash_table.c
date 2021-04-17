#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "common_util.h"
#include "hash_table.h"
#include "string_builder.h"
#include "number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
// 判定映射物件的型別用
#define VAL_TYPE_STR 0X0
#define VAL_TYPE_INT 0X1
#define VAL_TYPE_LONG 0X2
#define VAL_TYPE_FLOAT 0X3
#define VAL_TYPE_DOUBLE 0X4
// TODO: implment map
#define VAL_TYPE_MAP 0X5
// TODO: implment array
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

typedef struct
{
    char *key;
    HashItem *value;
    int type;
} HashTableItem;

struct HashTable_Private
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
    HashTableItem **items;
};

// 作為雜湊運算用的常數
static const int HASH_ARG = 0X11;
static const int DEFAULT_SIZE = 0X1f;
static const int DEFAULT_LOAD_FACTOR = 0X50;

// 作為判定已刪除的物件
static HashTableItem HT_DELETED_ITEM = {NULL, NULL, 0};

// 序列化用常數
static const char *QUOTE = "\"";
static const char *COLON = ":";
static const char *DELIMITER = ",";
static const char *BEGIN = "{";
static const char *END = "}";
static const char *INDENT = "  ";

// 判定序列化時，是否需要縮排
static const int NEED_INDENT = true;
static const int NO_NEED_INDENT = false;

static HashTableItem* _New_HashTableItem(const char *key, int val_type, HashItem *val)
{
    HashTableItem* item = (HashTableItem*) malloc(sizeof(HashTableItem));
    item->type = val_type;
    item->key = strdup(key);
    item->value = val;

    return item;
}

static HashTableItem* _New_Str_HashTableItem(const char *key, const char *val) 
{
    HashItem *str_value = (HashItem*) malloc(sizeof(HashItem));
    str_value->s_val = strdup(val);

    return _New_HashTableItem(key, VAL_TYPE_STR, str_value);
}

static HashTableItem* _New_Int_HashTableItem(const char *key, int val) 
{
    HashItem *value = (HashItem*) malloc(sizeof(HashItem));
    int *int_val = (int*) malloc(sizeof(int));
    *int_val = val;
    value->i_val = int_val;

    return _New_HashTableItem(key, VAL_TYPE_INT, value);
}

static HashTableItem* _New_Long_HashTableItem(const char *key, long val) 
{
    HashItem *value = (HashItem*) malloc(sizeof(HashItem));
    long *long_val = (long*) malloc(sizeof(long));
    *long_val = val;
    value->l_val = long_val;

    return _New_HashTableItem(key, VAL_TYPE_LONG, value);
}

static HashTableItem* _New_Double_HashTableItem(const char *key, double val) 
{
    HashItem *value = (HashItem*) malloc(sizeof(HashItem));
    double *double_val = (double*) malloc(sizeof(double));
    *double_val = val;
    value->d_val = double_val;

    return _New_HashTableItem(key, VAL_TYPE_DOUBLE, value);
}

static HashTableItem* _New_Float_HashTableItem(const char *key, float val) 
{
    HashItem *value = (HashItem*) malloc(sizeof(HashItem));
    float *float_val = (float*) malloc(sizeof(float));
    *float_val = val;
    value->f_val = float_val;

    return _New_HashTableItem(key, VAL_TYPE_FLOAT, value);
}

static void _Delete_HashTableItem(HashTableItem* item) 
{
    free(item->key);

    switch (item->type) 
    {
        case VAL_TYPE_STR:
            free(item->value->s_val);
            break;
        case VAL_TYPE_INT:
            free(item->value->i_val);
            break;
        case VAL_TYPE_LONG:
            free(item->value->l_val);
            break;
        case VAL_TYPE_DOUBLE:
            free(item->value->d_val);
            break;
        case VAL_TYPE_FLOAT:
            free(item->value->f_val);
            break;
    }

    free(item->value);
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
    int result = _Calculate_StringHash(str, HASH_ARG);
    if (result < 0) result = ~result;

    return (result + addition) % max_val;
}

static inline int _IsAbandoned(HashTableItem *item)
{
    return item == &HT_DELETED_ITEM;
}

static inline int _IsValid(HashTableItem *item)
{
    return item != NULL && !_IsAbandoned(item);
}

static HashTable* _New_HashTable(int size, int threshold)
{
    HashTable* table = calloc(1, sizeof(HashTable));
    HashTable_Private *priv = calloc(1, sizeof(HashTable_Private));

    priv->bucket_size = size;
    priv->resize_threshold = threshold;
    priv->item_count = 0;
    priv->modified_count = 0;
    priv->items = calloc((size_t) priv->bucket_size, sizeof(HashTableItem*));
    table->priv = priv;

    return table;
}

static void _AddItem(HashTable *table, HashTableItem *new_item)
{
    HashTable_Private *priv = table->priv;
    HashTableItem *item;
    int index, addition;
    
    addition = 0;
    while (true)
    {
        index = _Get_HashValue(new_item->key, priv->bucket_size, addition);
        item = priv->items[index];
        addition++;

        if (item == NULL) break;
        if (_IsAbandoned(item) || strcmp(new_item->key, item->key) != 0) continue;

        _Delete_HashTableItem(item);
        break;
    }

    priv->items[index] = new_item;
    priv->item_count++;
    priv->modified_count++;
}

static void _Resize(HashTable *table)
{
    HashTable_Private *curr_priv = table->priv;
    int current_size = curr_priv->item_count;
    int new_size = NumberUtil_NextPrime(current_size * 2);
    if (DEFAULT_SIZE >= new_size) 
    {
        new_size = DEFAULT_SIZE;
    }

    HashTable *temp_table = _New_HashTable(new_size, curr_priv->resize_threshold);
    if (!temp_table) 
    {
        s_out("malloc new HashTable failed...");
    }

    for (size_t i = 0; i < curr_priv->bucket_size; i++)
    {
        HashTableItem *item = curr_priv->items[i];
        if (!_IsValid(item)) continue;

        _AddItem(temp_table, item);
    }

    HashTable_Private *temp_priv = temp_table->priv;
    table->priv = temp_priv;
    free(curr_priv->items);

    if (curr_priv) 
    {
        free(curr_priv);
    }
}

static int _NeedResize(HashTable *table)
{
    HashTable_Private *priv = table->priv;
    return priv->modified_count * 100 / priv->bucket_size > priv->resize_threshold;
}

static inline void _EnsureBucketSize(HashTable *table)
{
    if (_NeedResize(table)) 
    {
        _Resize(table);
    }
}

static HashTableItem* _Find(HashTable *table, const char *key)
{
    HashTable_Private *priv = table->priv;
    HashTableItem *item;
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

static char* _ToJsonString(HashTable *table, int need_indent)
{
    int counter, depth;
    counter = 0;
    depth = 0;
    StringBuilder *builder = New_StringBuilder();
    StringBuilder_Append(builder, BEGIN);

    if (need_indent)
    {
        depth++;
    }

    HashTable_Private *priv = table->priv;
    for (size_t i = 0; i < priv->bucket_size; i++)
    {
        HashTableItem *item = priv->items[i];
        if (!_IsValid(item)) continue;
        if (counter > 0) 
        {
            StringBuilder_Append(builder, DELIMITER);
        }
        if (need_indent)
        {
            StringBuilder_Append(builder, "\n");
            for (size_t d = 0; d < depth; d++)
            {
                StringBuilder_Append(builder, "  ");
            }
        }
        StringBuilder_Append(builder, QUOTE);
        StringBuilder_Append(builder, item->key);
        StringBuilder_Append(builder, QUOTE);
        StringBuilder_Append(builder, COLON);
        
        int is_str = item->type == VAL_TYPE_STR;
        if (is_str) 
        {
            StringBuilder_Append(builder, QUOTE);
        }
        
        switch (item->type) 
        {
            case VAL_TYPE_STR:
                StringBuilder_Append(builder, item->value->s_val);
                break;
            case VAL_TYPE_INT:
                StringBuilder_Append(builder, *(item->value->i_val));
                break;
            case VAL_TYPE_LONG:
                StringBuilder_Append(builder, *(item->value->l_val));
                break;
            case VAL_TYPE_FLOAT:
                StringBuilder_Append(builder, *(item->value->f_val));
                break;
            case VAL_TYPE_DOUBLE:
                StringBuilder_Append(builder, *(item->value->d_val));
                break;
        }
        
        if (is_str) 
        {
            StringBuilder_Append(builder, QUOTE);
        }
        counter++;
    }

    if (need_indent)
    {
        StringBuilder_Append(builder, "\n");
        depth--;
        for (size_t d = 0; d < depth; d++)
        {
            StringBuilder_Append(builder, "  ");
        }
    }
    StringBuilder_Append(builder, END);
    char *str = StringBuilder_Value(builder);
    Delete_StringBuilder(&builder);
    return str;
}

// ================================================================================
// Public properties
// ================================================================================
HashTable* New_HashTable(void) 
{
    return _New_HashTable(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR);
}

HashTable* New_HashTable_WithBucketSize(int size) 
{
    return _New_HashTable(size, DEFAULT_LOAD_FACTOR);
}

HashTable* New_HashTable_WithBucketSizeAndLoadFactor(int size, int load_factor)
{
    return _New_HashTable(size, load_factor);
}

void Delete_HashTable(HashTable **p_to_table) 
{
    HashTable *table = *p_to_table;
    HashTable_Private *priv = table->priv;
    for (int i = 0; i < priv->bucket_size; i++) 
    {
        HashTableItem *item = priv->items[i];
        if (!_IsValid(item)) continue;

        _Delete_HashTableItem(item);
    }

    free(priv->items);
    free(priv);
    free(table);
    *p_to_table = NULL;
} 

void HashTable_Add_Str(HashTable *table, const char *key, const char *value)
{
    _EnsureBucketSize(table);
    HashTableItem *new_item = _New_Str_HashTableItem(key, value);
    _AddItem(table, new_item);
}

void HashTable_Add_Int(HashTable *table, const char *key, int value)
{
    _EnsureBucketSize(table);
    HashTableItem *new_item = _New_Int_HashTableItem(key, value);
    _AddItem(table, new_item);
}

void HashTable_Add_Long(HashTable *table, const char *key, long value)
{
    _EnsureBucketSize(table);
    HashTableItem *new_item = _New_Long_HashTableItem(key, value);
    _AddItem(table, new_item);
}

void HashTable_Add_Double(HashTable *table, const char *key, double value)
{
    _EnsureBucketSize(table);
    HashTableItem *new_item = _New_Double_HashTableItem(key, value);
    _AddItem(table, new_item);
}

void HashTable_Add_Float(HashTable *table, const char *key, float value)
{
    _EnsureBucketSize(table);
    HashTableItem *new_item = _New_Float_HashTableItem(key, value);
    _AddItem(table, new_item);
}

char* HashTable_Find_Str(HashTable *table, const char *key)
{
    HashTableItem *item = _Find(table, key);
    if (!item || item->type != VAL_TYPE_STR) return NULL;
    
    return item->value->s_val;
}

int* HashTable_Find_Int(HashTable *table, const char *key)
{
    HashTableItem *item = _Find(table, key);
    if (!item || item->type != VAL_TYPE_INT) return NULL;

    int *result = (item->value->i_val);
    return result;
}

long* HashTable_Find_Long(HashTable *table, const char *key)
{
    HashTableItem *item = _Find(table, key);
    if (!item || item->type != VAL_TYPE_LONG) return NULL;

    long *result = (item->value->l_val);
    return result;
}

double* HashTable_Find_Double(HashTable *table, const char *key)
{
    HashTableItem *item = _Find(table, key);
    if (!item || item->type != VAL_TYPE_DOUBLE) return NULL;

    double *result = (item->value->d_val);
    return result;
}

float* HashTable_Find_Float(HashTable *table, const char *key)
{
    HashTableItem *item = _Find(table, key);
    if (!item || item->type != VAL_TYPE_FLOAT) return NULL;

    float *result = (item->value->f_val);
    return result;
}

void HashTable_Delete(HashTable *table, const char *key)
{
    HashTable_Private *priv = table->priv;
    HashTableItem *item;
    int index, addition;

    addition = 0;
    while (true)
    {
        index = _Get_HashValue(key, priv->bucket_size, addition);
        item = priv->items[index];
        addition++;

        if (item == NULL) break;
        if (_IsAbandoned(item) || strcmp(item->key, key) != 0) continue;

        _Delete_HashTableItem(item);
        priv->items[index] = &HT_DELETED_ITEM;
        priv->item_count--;
        priv->modified_count++;
        break;
    }

    _EnsureBucketSize(table);
}

int HashTable_Size(HashTable *table)
{
    return table->priv->item_count;
}

inline bool HashTable_IsEmpty(HashTable *table)
{
    return HashTable_Size(table) == 0;
}

char* HashTable_ToJsonStr(HashTable *table)
{
    return _ToJsonString(table, NO_NEED_INDENT);
}

char* HashTable_ToIndentJsonStr(HashTable *table)
{
    return _ToJsonString(table, NEED_INDENT);
}

