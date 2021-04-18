#ifndef GENERIC_JSON_SERIALIZER_H
#define GENERIC_JSON_SERIALIZER_H

struct GenericTable;
struct GenericList;

#define JsonSerializer_ToStr(var) _Generic((var),\
    struct GenericTable*: JsonSerializer_TableToStr,\
    struct GenericList*: JsonSerializer_ListToStr\
) (var)

/**
 * 將映射表輸出成 JSON 字串
 */
char* JsonSerializer_TableToStr(struct GenericTable *table);

/**
 * 將動態陣列輸出成 JSON 字串
 */
char* JsonSerializer_ListToStr(struct GenericList *list);

#define JsonSerializer_ToIndentStr(var) _Generic((var),\
    struct GenericTable*: JsonSerializer_TableToIndentStr,\
    struct GenericList*: JsonSerializer_ListToIndentStr\
) (var)

/**
 * 將映射表輸出成有縮排的 JSON 字串
 */
char* JsonSerializer_TableToIndentStr(struct GenericTable *table);

/**
 * 將動態陣列輸出成有縮排的 JSON 字串
 */
char* JsonSerializer_ListToIndentStr(struct GenericList *list);

#endif