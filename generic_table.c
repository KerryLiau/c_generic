#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "common_util.h"
#include "generic_table.h"
#include "generic_type.h"
#include "string_builder.h"
#include "number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
typedef struct GenericTableItem
{
    char *key;
    GenericType *value;
} GenericTableItem;

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

// 作為雜湊運算用的常數
static const int HASH_ARG = 0X11;
static const int DEFAULT_SIZE = 0X1f;
static const int DEFAULT_LOAD_FACTOR = 0X50;

// 作為判定已刪除的物件
static GenericTableItem HT_DELETED_ITEM = {NULL, NULL};

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

static GenericTableItem* _New_GenericTableItem(const char *key, GenericType *val)
{
    GenericTableItem* item = (GenericTableItem*) malloc(sizeof(GenericTableItem));
    item->key = strdup(key);
    item->value = val;

    return item;
}

static GenericTableItem* _New_Str_GenericTableItem(const char *key, const char *val) 
{
    GenericType *gen_obj = New_GenericType(val);
    return _New_GenericTableItem(key, gen_obj);
}

static GenericTableItem* _New_Int_GenericTableItem(const char *key, int val) 
{
    GenericType *gen_obj = New_GenericType(val);
    return _New_GenericTableItem(key, gen_obj);
}

static GenericTableItem* _New_Long_GenericTableItem(const char *key, long val) 
{
    GenericType *gen_obj = New_GenericType(val);
    return _New_GenericTableItem(key, gen_obj);
}

static GenericTableItem* _New_Double_GenericTableItem(const char *key, double val) 
{
    GenericType *gen_obj = New_GenericType(val);
    return _New_GenericTableItem(key, gen_obj);
}

static GenericTableItem* _New_Float_GenericTableItem(const char *key, float val) 
{
    GenericType *gen_obj = New_GenericType(val);
    return _New_GenericTableItem(key, gen_obj);
}

static GenericTableItem* _New_Table_GenericTableItem(const char *key, GenericTable *val)
{
    GenericType *gen_obj = New_GenericType(val);
    return _New_GenericTableItem(key, gen_obj);
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
    int result = _Calculate_StringHash(str, HASH_ARG);
    if (result < 0) result = ~result;

    return (result + addition) % max_val;
}

static inline int _IsAbandoned(GenericTableItem *item)
{
    return item == &HT_DELETED_ITEM;
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

static void _Resize(GenericTable *table)
{
    GenericTable_Private *curr_priv = table->priv;
    int current_size = curr_priv->item_count;
    int new_size = NumberUtil_NextPrime(current_size * 2);
    if (DEFAULT_SIZE >= new_size) 
    {
        new_size = DEFAULT_SIZE;
    }

    GenericTable *temp_table = _New_GenericTable(new_size, curr_priv->resize_threshold);
    if (!temp_table) 
    {
        s_out("malloc new GenericTable failed...");
    }

    for (size_t i = 0; i < curr_priv->bucket_size; i++)
    {
        GenericTableItem *item = curr_priv->items[i];
        if (!_IsValid(item)) continue;

        _AddItem(temp_table, item);
    }

    GenericTable_Private *temp_priv = temp_table->priv;
    table->priv = temp_priv;
    free(curr_priv->items);

    if (curr_priv) 
    {
        free(curr_priv);
    }
}

static int _NeedResize(GenericTable *table)
{
    GenericTable_Private *priv = table->priv;
    return priv->modified_count * 100 / priv->bucket_size > priv->resize_threshold;
}

static inline void _EnsureBucketSize(GenericTable *table)
{
    if (_NeedResize(table)) 
    {
        _Resize(table);
    }
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

static char* _ToJsonString(GenericTable *table, int level, bool need_indent)
{
    int counter, depth;
    counter = 0;
    depth = level;
    StringBuilder *builder = New_StringBuilder();
    StringBuilder_Append(builder, BEGIN);

    if (need_indent)
    {
        depth++;
    }

    GenericTable_Private *priv = table->priv;
    for (size_t i = 0; i < priv->bucket_size; i++)
    {
        GenericTableItem *item = priv->items[i];
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

        GenericType *gen = item->value;
        GenericTypeEnum type = GenericType_GetType(gen);
        bool is_str = type == GEN_TYPE_STR;
        if (is_str) 
        {
            StringBuilder_Append(builder, QUOTE);
        }
        
        switch (type) 
        {
            case GEN_TYPE_STR:
                StringBuilder_Append(builder, GenericType_GetStr(gen));
                break;
            case GEN_TYPE_INT:
                StringBuilder_Append(builder, *GenericType_GetInt(gen));
                break;
            case GEN_TYPE_LONG:
                StringBuilder_Append(builder, *GenericType_GetLong(gen));
                break;
            case GEN_TYPE_FLOAT:
                StringBuilder_Append(builder, *GenericType_GetFloat(gen));
                break;
            case GEN_TYPE_DOUBLE:
                StringBuilder_Append(builder, *GenericType_GetDouble(gen));
                break;
            case GEN_TYPE_TABLE:
            {
                // should extract as json serializer?
                char *map_str = _ToJsonString(GenericType_GetTable(gen), level + 1, need_indent);
                StringBuilder_Append(builder, map_str);
                free(map_str);
                break;
            }
            case GEN_TYPE_LIST:
                // todo
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
GenericTable* New_GenericTable(void) 
{
    return _New_GenericTable(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR);
}

GenericTable* New_GenericTable_WithBucketSize(int size) 
{
    return _New_GenericTable(size, DEFAULT_LOAD_FACTOR);
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
    GenericTableItem *new_item = _New_Str_GenericTableItem(key, value);
    _AddItem(table, new_item);
}

void GenericTable_Add_Int(GenericTable *table, const char *key, int value)
{
    _EnsureBucketSize(table);
    GenericTableItem *new_item = _New_Int_GenericTableItem(key, value);
    _AddItem(table, new_item);
}

void GenericTable_Add_Long(GenericTable *table, const char *key, long value)
{
    _EnsureBucketSize(table);
    GenericTableItem *new_item = _New_Long_GenericTableItem(key, value);
    _AddItem(table, new_item);
}

void GenericTable_Add_Double(GenericTable *table, const char *key, double value)
{
    _EnsureBucketSize(table);
    GenericTableItem *new_item = _New_Double_GenericTableItem(key, value);
    _AddItem(table, new_item);
}

void GenericTable_Add_Float(GenericTable *table, const char *key, float value)
{
    _EnsureBucketSize(table);
    GenericTableItem *new_item = _New_Float_GenericTableItem(key, value);
    _AddItem(table, new_item);
}

void GenericTable_Add_Table(GenericTable *table, const char *key, GenericTable *value)
{
    _EnsureBucketSize(table);
    GenericTableItem *new_item = _New_Table_GenericTableItem(key, value);
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
        priv->items[index] = &HT_DELETED_ITEM;
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

char* GenericTable_ToJsonStr(GenericTable *table)
{
    return _ToJsonString(table, 0, NO_NEED_INDENT);
}

char* GenericTable_ToIndentJsonStr(GenericTable *table)
{
    return _ToJsonString(table, 0, NEED_INDENT);
}

