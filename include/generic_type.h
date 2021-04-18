#ifndef GENERIC_TYPE_H
#define GENERIC_TYPE_H

#include "stdlib.h"
#include "generic_type_enum.h"
#include "generic_table.h"
#include "common_util.h"

typedef struct GenericType GenericType;

void Delete_GenericType(GenericType **ptr_obj);

struct GenericList;// prevent recursive import

#define New_GenericType(val) _Generic((val), \
    char*: New_Str_GenericType,\
    const char*: New_Str_GenericType,\
    int: New_Int_GenericType,\
    long: New_Long_GenericType,\
    float: New_Float_GenericType,\
    double: New_Double_GenericType,\
    GenericTable*: New_Table_GenericType,\
    struct GenericList*: New_List_GenericType\
    ) (val)

GenericType* New_Str_GenericType(const char *value);

GenericType* New_Int_GenericType(int value);

GenericType* New_Long_GenericType(long value);

GenericType* New_Float_GenericType(float value);

GenericType* New_Double_GenericType(double value);

GenericType* New_Table_GenericType(GenericTable *value);

GenericType* New_List_GenericType(struct GenericList *value);


char* GenericType_GetStr(GenericType *gen_type);

int* GenericType_GetInt(GenericType *gen_type);

long* GenericType_GetLong(GenericType *gen_type);

float* GenericType_GetFloat(GenericType *gen_type);

double* GenericType_GetDouble(GenericType *gen_type);

GenericTable* GenericType_GetTable(GenericType *gen_type);

struct GenericList* GenericType_GetList(GenericType *gen_type);

GenericTypeEnum GenericType_GetType(GenericType *gen_type);

bool GenericType_IsType(GenericType *gen_type, GenericTypeEnum type);

bool GenericType_Equals(GenericType *gen_type1, GenericType *gen_type2);

#endif 