#ifndef GENERIC_TABLE_H
#define GENERIC_TABLE_H

#include "common_util.h"

struct GenericList;

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
 * GenericTableItem **items:
 * 承裝映射物件的容器
 */
typedef struct GenericTable_Private GenericTable_Private;

typedef struct GenericTableItem GenericTableItem;

typedef struct GenericTable
{
    GenericTable_Private *priv;
} GenericTable;

/**
 * 建構映射表，大小、負載係數都是預設的
 */
GenericTable* New_GenericTable(void);

/**
 * 建構指定預設大小的映射表
 */
GenericTable* New_GenericTable_WithBucketSize(int size);

/**
 * 建構指定預設大小、負載係數的映射表
 */
GenericTable* New_GenericTable_WithBucketSizeAndLoadFactor(int size, int load_factor);

/**
 * 解構映射表
 * **table: 映射表自身的位址指標 ex: &table
 */
void Delete_GenericTable(GenericTable **table);

/**
 * 映射表新增物件的泛型方法，
 * 當 key 已存在時，會更新原本映射的物件
 */
#define GenericTable_Add(table, key, value) _Generic((value),\
    const char*: GenericTable_Add_Str,\
    char*: GenericTable_Add_Str,\
    int: GenericTable_Add_Int,\
    long: GenericTable_Add_Long,\
    double: GenericTable_Add_Double,\
    float: GenericTable_Add_Float,\
    GenericTable*: GenericTable_Add_Table,\
    struct GenericList*: GenericTable_Add_List\
)(table, key, value)

void GenericTable_Add_Str(GenericTable *table, const char *key, const char *value);

void GenericTable_Add_Int(GenericTable *table, const char *key, int value);

void GenericTable_Add_Long(GenericTable *table, const char *key, long value);

void GenericTable_Add_Double(GenericTable *table, const char *key, double value);

void GenericTable_Add_Float(GenericTable *table, const char *key, float value);

void GenericTable_Add_Table(GenericTable *table, const char *key, GenericTable *value);

void GenericTable_Add_List(GenericTable *table, const char *key, struct GenericList *value);

/**
 * 在映射表中查找字串指標，
 * 如 key 不存在，或查找出的值並非字串，將回傳 NULL
 */
char* GenericTable_Find_Str(GenericTable *table, const char *key);

/**
 * 在映射表中查找整數指標，
 * 如 key 不存在，或查找出的值並非整數，將回傳 NULL
 */
int* GenericTable_Find_Int(GenericTable *table, const char *key);

/**
 * 在映射表中查找長整數指標，
 * 如 key 不存在，或查找出的值並非長整數，將回傳 NULL
 */
long* GenericTable_Find_Long(GenericTable *table, const char *key);

/**
 * 在映射表中查找雙經度浮點數指標，
 * 如 key 不存在，或查找出的值並非雙經度浮點數，將回傳 NULL
 */
double* GenericTable_Find_Double(GenericTable *table, const char *key);

/**
 * 在映射表中查找單經度浮點數指標，
 * 如 key 不存在，或查找出的值並非單經度浮點數，將回傳 NULL
 */
float* GenericTable_Find_Float(GenericTable *table, const char *key);

/**
 * 在映射表中查找映射表，
 * 如 key 不存在，或查找出的值並非映射表，將回傳 NULL
 */
GenericTable* GenericTable_Find_Table(GenericTable *table, const char *key);

/**
 * 移除映射表中 key 對應到的物件，並將對應的位置標記為已棄用
 */
void GenericTable_Delete(GenericTable *table, const char *key);

/**
 * 在映射表中查找 key 是否存在
 */
bool GenericTable_HasKey(GenericTable *table, const char *key);

/**
 * 取得映射表當前物件數量
 */
int GenericTable_Size(GenericTable *table);

/**
 * 得知映射表當前是否無任何物件，
 * 0 = false, 1 = true
 */
bool GenericTable_IsEmpty(GenericTable *table);

bool GenericTableItem_IsValid(GenericTableItem *item);

GenericTableItem** GenericTable_GetItems(GenericTable *table);

int GenericTable_GetBucketSize(GenericTable *table);

struct GenericType;

char* GenericTableItem_GetKey(GenericTableItem *item);

struct GenericType* GenericTableItem_GetValue(GenericTableItem *item);

#endif