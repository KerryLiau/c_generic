#ifndef GENERIC_LIST_H
#define GENERIC_LIST_H

#include "generic_table.h"

typedef struct GenericList GenericList;

struct GenericType;

GenericList* New_GenericList();

void Delete_GenericList(GenericList **p_list);

struct GenericType* GenericList_At(GenericList *list, int index);

int GenericList_Size(GenericList *list);

bool GenericList_IsEmpty(GenericList *list);

#define GenericList_Add(list, val) _Generic((val),\
    char*: GenericList_Add_Str,\
    const char*: GenericList_Add_Str,\
    int: GenericList_Add_Int,\
    long: GenericList_Add_Long,\
    float: GenericList_Add_Float,\
    double: GenericList_Add_Double,\
    GenericTable*: GenericList_Add_Table,\
    GenericList*: GenericList_Add_List\
) (list, val)

void GenericList_Add_Str(GenericList *list, char *val);

void GenericList_Add_Int(GenericList *list, int val);

void GenericList_Add_Long(GenericList *list, long val);

void GenericList_Add_Float(GenericList *list, float val);

void GenericList_Add_Double(GenericList *list, double val);

void GenericList_Add_Table(GenericList *list, GenericTable *val);

void GenericList_Add_List(GenericList *list, GenericList *val);

/**
 * return: the operate is success or not
 */
bool GenericList_DeleteAt(GenericList *list, int index);

void GenericList_IndexAt(GenericList *list, struct GenericType *obj);

#endif