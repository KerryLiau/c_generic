#include "generic_list.h"
#include "common_util.h"
#include "generic_type.h"
#include <stdlib.h>
#include <string.h>

static const int DEFAULT_SIZE = 0x10;

struct GenericList
{
    int size;
    int max_size;
    GenericType **elements;
};

static GenericList* _New_GenericList(int init_size)
{
    GenericList *list = (GenericList*) malloc(sizeof(GenericList));
    list->size = 0;
    list->max_size = init_size;

    GenericType **elements = (GenericType**) calloc(init_size, sizeof(GenericList));
    list->elements = elements;

    return list;
}

GenericList* New_GenericList()
{
    return _New_GenericList(DEFAULT_SIZE);
}

void Delete_GenericList(GenericList **list)
{
    s_out("Delete_GenericList");
}

GenericType* GenericList_At(int index);

int GenericList_Size(GenericList *list);

bool GenericList_IsEmpty(GenericList *list);

void GenericList_Add_Str(GenericList *list, char *val);

void GenericList_Add_Int(GenericList *list, int val);

void GenericList_Add_Long(GenericList *list, long val);

void GenericList_Add_Float(GenericList *list, float val);

void GenericList_Add_Double(GenericList *list, double val);

void GenericList_Add_Table(GenericList *list, GenericTable *val);

void GenericList_Add_List(GenericList *list, GenericList *val);

void GenericList_DeleteAt(GenericList *list, int index);

void GenericList_IndexAt(GenericList *list, GenericType *obj);