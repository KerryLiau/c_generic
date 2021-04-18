#include "../include/json_serializer.h"
#include "../include/common_util.h"
#include "../include/generic_list.h"
#include "../include/generic_type_enum.h"
#include "../include/string_builder.h"
#include "../include/generic_table.h"
#include "../include/generic_type.h"

// 序列化用常數
static const char *QUOTE = "\"";
static const char *COLON = ":";
static const char *DELIMITER = ",";
static const char *OBJECT_BEGIN = "{";
static const char *OBJECT_END = "}";
static const char *INDENT = "  ";
static const char *ARRAY_BEGIN = "[";
static const char *ARRAY_END = "]";

// 判定序列化時，是否需要縮排
static const int NEED_INDENT = true;
static const int NO_NEED_INDENT = false;

static void _SerializeContent_Callback(
    StringBuilder *builder, GenericTypeEnum type, GenericType *gen, int level, bool need_indent,
    char* (_Callback) (GenericTypeEnum type, void *items, int items_count, int level, bool need_indent)
) {
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
            GenericTable *gen_table = GenericType_GetTable(gen);
            GenericTableIterator *iterator = GenericTable_GetIterator(gen_table);
            char *map_str = _Callback(GEN_TYPE_TABLE, iterator, 0, level + 1, need_indent);
            StringBuilder_Append(builder, map_str);
            free(map_str);
            Delete_GenericTableIterator(&iterator);
            break;
        }
        case GEN_TYPE_LIST:
        {
            struct GenericList *gen_list = GenericType_GetList(gen);
            int gen_list_size = GenericList_Size(gen_list);
            char *list_str = _Callback(GEN_TYPE_LIST, gen_list, gen_list_size, level + 1, need_indent);
            StringBuilder_Append(builder, list_str);
            free(list_str);
            break;
        }
    }
}

static char* _Object_ToJsonString(GenericTypeEnum type, void *items, int items_count, int level, bool need_indent)
{
    int counter, depth;
    counter = 0;
    depth = level;
    bool is_table = false;
    if (type == GEN_TYPE_TABLE) is_table = true;

    StringBuilder *builder = New_StringBuilder();
    if (is_table)
        StringBuilder_Append(builder, OBJECT_BEGIN);
    else 
        StringBuilder_Append(builder, ARRAY_BEGIN);
    if (need_indent) depth++;

    GenericTableIterator *iterator = NULL;
    if (is_table) 
        iterator = (GenericTableIterator*) items;
    GenericTableItem *table_item = NULL;
    GenericType *array_item = NULL;
    int index = 0;
    bool keep_going;
    if (is_table)
        keep_going = GenericTableIterator_HasNext((GenericTableIterator*) items);
    else
        keep_going = index < items_count;
    while (keep_going)
    {
        if (is_table)
            table_item = (GenericTableItem*) GenericTableIterator_Next(iterator);
        else 
            array_item = (GenericType*) GenericList_At(((GenericList*) items), index);
        if (counter > 0) 
            StringBuilder_Append(builder, DELIMITER);
        if (need_indent)
        {
            StringBuilder_Append(builder, "\n");
            for (size_t d = 0; d < depth; d++)
                StringBuilder_Append(builder, "  ");
        }

        if (is_table)
        {
            StringBuilder_Append(builder, QUOTE);
            StringBuilder_Append(builder, GenericTableItem_GetKey(table_item));
            StringBuilder_Append(builder, QUOTE);
            StringBuilder_Append(builder, COLON);
        }
        
        struct GenericType *gen;
        if (is_table)
            gen = GenericTableItem_GetValue(table_item);
        else
            gen = array_item;
        GenericTypeEnum type = GenericType_GetType(gen);
        bool is_str = type == GEN_TYPE_STR;
        if (is_str) 
            StringBuilder_Append(builder, QUOTE);

        _SerializeContent_Callback(builder, type, gen, level, need_indent, _Object_ToJsonString);
        
        if (is_str) 
            StringBuilder_Append(builder, QUOTE);
        counter++;
        index++;
        if (is_table)
            keep_going = GenericTableIterator_HasNext(iterator);
        else
            keep_going = index < items_count;
    }

    if (need_indent)
    {
        StringBuilder_Append(builder, "\n");
        depth--;
        for (size_t d = 0; d < depth; d++)
            StringBuilder_Append(builder, "  ");
    }
    if (is_table)
        StringBuilder_Append(builder, OBJECT_END);
    else 
        StringBuilder_Append(builder, ARRAY_END);
    char *str = StringBuilder_Value(builder);
    Delete_StringBuilder(&builder);
    return str;
}

char* JsonSerializer_TableToStr(struct GenericTable *table)
{
    GenericTableIterator *iterator = GenericTable_GetIterator(table);
    char *json_str = _Object_ToJsonString(GEN_TYPE_TABLE, iterator, 0, 0, NO_NEED_INDENT);
    Delete_GenericTableIterator(&iterator);
    return json_str;
}

char* JsonSerializer_TableToIndentStr(struct GenericTable *table)
{
    GenericTableIterator *iterator = GenericTable_GetIterator(table);
    char *json_str = _Object_ToJsonString(GEN_TYPE_TABLE, iterator, 0, 0, NEED_INDENT);
    Delete_GenericTableIterator(&iterator);
    return json_str;
}

char* JsonSerializer_ListToStr(struct GenericList *list)
{
    int list_size = GenericList_Size(list);
    return _Object_ToJsonString(GEN_TYPE_LIST, list, list_size, 0, NO_NEED_INDENT);
}

char* JsonSerializer_ListToIndentStr(struct GenericList *list)
{
    int list_size = GenericList_Size(list);
    return _Object_ToJsonString(GEN_TYPE_LIST, list, list_size, 0, NEED_INDENT);
}




