#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common_util.h"

/**
 * 映射表的私有屬性，裡面的屬性：
 * 
 * int bucket_size:
 * 雜湊映射表的容器大小
 *
 * int resize_threshold:
 * 重構臨界點，用以判定容器是否擴充、縮減
 *
 * int item_count:
 * 實際存在的映射物件(被標記為'已棄用'物件不算在內)，會隨著刪除方法而減少
 *
 * int modified_count:
 * 成功新增、刪除的計數，不會隨著刪除方法而減少
 *
 * HashTableItem **items:
 * 承裝映射物件的容器
 */
typedef struct HashTable_Private HashTable_Private;

typedef struct 
{
    HashTable_Private *priv;
} HashTable;

/**
 * 建構映射表，大小、重構臨界點都是預設的
 */
HashTable* New_HashTable(void);

/**
 * 建構指定預設大小的映射表
 */
HashTable* New_HashTable_WithBucketSize(int size);

/**
 * 建構指定預設大小、重構臨界點的映射表
 */
HashTable* New_HashTable_WithBucketSizeAndThreshold(int size, int threshold);

/**
 * 解構映射表
 * **table: 映射表自身的位址指標 ex: &table
 */
void Delete_HashTable(HashTable **table);

/**
 * 映射表新增物件的泛型方法，
 * 當 key 已存在時，會更新原本映射的物件
 */
#define HashTable_Add(table, key, value) _Generic((value),\
    const char*: HashTable_Add_Str,\
    char*: HashTable_Add_Str,\
    int: HashTable_Add_Int,\
    long: HashTable_Add_Long,\
    double: HashTable_Add_Double,\
    float: HashTable_Add_Float\
)(table, key, value)

void HashTable_Add_Str(HashTable *table, const char *key, const char *value);

void HashTable_Add_Int(HashTable *table, const char *key, int value);

void HashTable_Add_Long(HashTable *table, const char *key, long value);

void HashTable_Add_Double(HashTable *table, const char *key, double value);

void HashTable_Add_Float(HashTable *table, const char *key, float value);

/**
 * 在映射表中查找字串指標，
 * 如 *key 不存在，或查找出的值並非字串，將回傳 NULL
 */
char* HashTable_Find_Str(HashTable *table, const char *key);

/**
 * 在映射表中查找整數指標，
 * 如 *key 不存在，或查找出的值並非整數，將回傳 NULL
 */
int* HashTable_Find_Int(HashTable *table, const char *key);

/**
 * 在映射表中查找長整數指標，
 * 如 *key 不存在，或查找出的值並非長整數，將回傳 NULL
 */
long* HashTable_Find_Long(HashTable *table, const char *key);

/**
 * 在映射表中查找雙經度浮點數指標，
 * 如 *key 不存在，或查找出的值並非雙經度浮點數，將回傳 NULL
 */
double* HashTable_Find_Double(HashTable *table, const char *key);

/**
 * 在映射表中查找單經度浮點數指標，
 * 如 *key 不存在，或查找出的值並非單經度浮點數，將回傳 NULL
 */
float* HashTable_Find_Float(HashTable *table, const char *key);

/**
 * 移除映射表中 *key 對應到的物件，並將對應的位置標記為已棄用
 */
void HashTable_Delete(HashTable *table, const char *key);

/**
 * 將映射表輸出成 JSON 字串
 */
char* HashTable_ToJsonStr(HashTable *table);

/**
 * 將映射表輸出成有縮排的 JSON 字串
 */
char* HashTable_ToIndentJsonStr(HashTable *table);

/**
 * 取得映射表當前物件數量
 */
int HashTable_Size(HashTable *table);

/**
 * 得知映射表當前是否無任何物件，
 * 0 = false, 1 = true
 */
bool HashTable_IsEmpty(HashTable *table);

#endif