#include <stdlib.h>
#include <string.h>

#include "../include/generic_list.h"
#include "../include/string_builder.h"
#include "../include/common_util.h"
#include "../include/generic_type.h"
#include "../include/number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
static const int DEFAULT_SIZE = 0x10;

static const char *QUOTE = "\"";
static const char *COLON = ":";
static const char *DELIMITER = ",";
static const char *BEGIN = "{";
static const char *END = "}";
static const char *INDENT = "  ";

struct GenericList
{
    int next;
    int max_size;
    GenericType **elements;
};

static GenericList* _New_GenericList(int init_size)
{
    GenericList *list = (GenericList*) malloc(sizeof(GenericList));
    list->next = 0;
    list->max_size = init_size;

    GenericType **elements = (GenericType**) calloc(init_size, sizeof(GenericType*));
    list->elements = elements;

    return list;
}

inline static bool _NeedResize(GenericList *list, int num)
{
    return list->next + num > list->max_size;
}

static void _EnsureSize(GenericList *list, int num)
{
    if (!_NeedResize(list, num)) return;
    
    int curr_max = list->max_size;
    int new_max = curr_max | (curr_max >> 1);
    if (new_max > NUMBER_UTIL_INT_MAX) 
    {
        s_out("list size is over integer max");
        return;
    }

    GenericType **new_element = (GenericType**) calloc(new_max, sizeof(GenericType*));
    for (int i = 0; i < list->next; i++) 
    {
        new_element[i] = list->elements[i];
    }

    free(list->elements);
    list->elements = new_element;
    list->max_size = new_max;
}

static void _AddSingle(GenericList *list, GenericType *gen)
{
    _EnsureSize(list, 1);
    list->elements[list->next] = gen;
    list->next++;
}

static void _AddAll(GenericList *list, GenericType **gen_list, int gen_count)
{
    _EnsureSize(list, gen_count);
    for (int i = 0; i < gen_count; i++)
    {
        list->elements[list->next] = gen_list[i];
        list->next++;
    }
}

// ================================================================================
// Public properties
// ================================================================================
GenericList* New_GenericList()
{
    return _New_GenericList(DEFAULT_SIZE);
}

void Delete_GenericList(GenericList **p_list)
{
    GenericList *list = *p_list;
    for (int i = 0; i < list->next; i++)
    {
        Delete_GenericType(&(list->elements[i]));
    }
    free(list->elements);
    free(list);
    *p_list = NULL;
}

GenericType* GenericList_At(GenericList *list, int index)
{
    return list->elements[index];
}

int GenericList_Size(GenericList *list)
{
    return list->next;
}

bool GenericList_IsEmpty(GenericList *list)
{
    return list->next == 0;
}

void GenericList_Add_Str(GenericList *list, char *val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

void GenericList_Add_Int(GenericList *list, int val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

void GenericList_Add_Long(GenericList *list, long val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

void GenericList_Add_Float(GenericList *list, float val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

void GenericList_Add_Double(GenericList *list, double val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

void GenericList_Add_Table(GenericList *list, GenericTable *val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

void GenericList_Add_List(GenericList *list, GenericList *val)
{
    GenericType *gen = New_GenericType(val);
    _AddSingle(list, gen);
}

bool GenericList_DeleteAt(GenericList *list, int index)
{
    if (index >= list->next) 
    {
        s_out_err_f("index '%d' is out of bound '%d'", index, list->next);
        return false;
    }

    GenericType *gen = list->elements[index];
    Delete_GenericType(&gen);
    if (index == list->next - 1) 
    {
        list->next--;
        return true;
    }

    // move right side
    for (int i = index; i < list->next - 1; i++)
    {
        list->elements[i] = list->elements[i + 1];
    }
    list->elements[list->next - 1] = NULL;
    list->next--;
    return true;
}

void GenericList_IndexAt(GenericList *list, GenericType *obj);






