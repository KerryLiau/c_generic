#include "generic_type.h"
#include "generic_list.h"
#include "common_util.h"
#include "generic_type_enum.h"
#include <string.h>
#include <stdlib.h>

// ================================================================================
// Private Properties
// ================================================================================
typedef union GenericValue
{
    char *s_val;
    int *i_val;
    long *l_val;
    float *f_val;
    double *d_val;
    GenericTable *h_val;
    GenericList *a_val;
} GenericValue;

struct GenericType
{
    GenericTypeEnum type;
    GenericValue *value;
};

static GenericType* _New_GenericType(GenericTypeEnum type, void *value)
{
    GenericType *gen_obj = (GenericType*) malloc(sizeof(GenericType));
    GenericValue *gen_val = (GenericValue*) malloc(sizeof(GenericValue));
    gen_obj->type = type;
    gen_obj->value = gen_val;

    switch (type)
    {
        case GEN_TYPE_STR:
            gen_val->s_val = (char*) value;
            break;
        case GEN_TYPE_INT:
            gen_val->i_val = (int*) value;
            break;
        case GEN_TYPE_LONG:
            gen_val->l_val = (long*) value;
            break;
        case GEN_TYPE_DOUBLE:
            gen_val->d_val = (double*) value;
            break;
        case GEN_TYPE_FLOAT:
            gen_val->f_val = (float*) value;
            break;
        case GEN_TYPE_TABLE:
            gen_val->h_val = (GenericTable*) value;
            break;
        case GEN_TYPE_LIST:
            gen_val->a_val = (GenericList*) value;
            break;
    }

    return gen_obj;
}

// ================================================================================
// Public properties
// ================================================================================
void Delete_GenericType(GenericType **ptr_obj)
{
    GenericType *obj = *ptr_obj;
    GenericValue *gen_val = obj->value;
    switch (obj->type)
    {
        case GEN_TYPE_STR:
            free(gen_val->s_val);
            break;
        case GEN_TYPE_INT:
            free(gen_val->i_val);
            break;
        case GEN_TYPE_LONG:
            free(gen_val->l_val);
            break;
        case GEN_TYPE_DOUBLE:
            free(gen_val->d_val);
            break;
        case GEN_TYPE_FLOAT:
            free(gen_val->f_val);
            break;
        case GEN_TYPE_TABLE:
            Delete_GenericTable(&(gen_val->h_val));
            break;
        case GEN_TYPE_LIST:
            Delete_GenericList(&(gen_val->a_val));
            break;
    }
    free(gen_val);
    free(obj);
    *ptr_obj = NULL;
}

GenericType* New_Str_GenericType(const char *value)
{
    if (!value) 
    {
        s_out("the string pointer is null");
        return NULL;
    }
    return _New_GenericType(GEN_TYPE_STR, strdup(value));
}

GenericType* New_Int_GenericType(int value)
{
    int *p_val = (int*) malloc(sizeof(int));
    *p_val = value;
    return _New_GenericType(GEN_TYPE_INT, p_val);
}

GenericType* New_Long_GenericType(long value)
{
    long *p_val = (long*) malloc(sizeof(long));
    *p_val = value;
    return _New_GenericType(GEN_TYPE_LONG, p_val);
}

GenericType* New_Float_GenericType(float value)
{
    float *p_val = (float*) malloc(sizeof(float));
    *p_val = value;
    return _New_GenericType(GEN_TYPE_FLOAT, p_val);
}

GenericType* New_Double_GenericType(double value)
{
    double *p_val = (double*) malloc(sizeof(double));
    *p_val = value;
    return _New_GenericType(GEN_TYPE_DOUBLE, p_val);
}

GenericType* New_Table_GenericType(GenericTable *value)
{
    if (!value) 
    {
        s_out("the GenericTable pointer is null");
        return NULL;
    }
    return _New_GenericType(GEN_TYPE_TABLE, value);
}

GenericType* New_List_GenericType(struct GenericList *value)
{
    if (!value) 
    {
        s_out("the GenericList pointer is null");
        return NULL;
    }
    return _New_GenericType(GEN_TYPE_LIST, value);
}

char* GenericType_GetStr(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_STR) return NULL;
    return gen_type->value->s_val;
}

int* GenericType_GetInt(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_INT) return NULL;
    return gen_type->value->i_val;
}

long* GenericType_GetLong(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_LONG) return NULL;
    return gen_type->value->l_val;
}

float* GenericType_GetFloat(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_FLOAT) return NULL;
    return gen_type->value->f_val;
}

double* GenericType_GetDouble(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_DOUBLE) return NULL;
    return gen_type->value->d_val;
}

GenericTable* GenericType_GetTable(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_TABLE) return NULL;
    return gen_type->value->h_val;
}

struct  GenericList* GenericType_GetList(GenericType *gen_type)
{
    if (gen_type->type != GEN_TYPE_LIST) return NULL;
    return gen_type->value->a_val;
}

GenericTypeEnum GenericType_GetType(GenericType *gen_type)
{
    return gen_type->type;
}

bool GenericType_IsType(GenericType *gen_type, GenericTypeEnum type)
{
    return gen_type->type == type;
}

bool GenericType_Equals(GenericType *gen_type1, GenericType *gen_type2)
{
    if (CommonUtil_IsNull(gen_type1) | CommonUtil_IsNull(gen_type2)) 
    {
        if (!gen_type1) 
        {
            s_out_err("first param is null at GenericType_Equals!");
        }
        if (!gen_type2) 
        {
            s_out_err("second param is null at GenericType_Equals!");
        }
        return false;
    }
    if (gen_type1 == gen_type2) return true;
    if (gen_type1->type != gen_type2->type) return false;

    GenericValue *val1, *val2;
    val1 = gen_type1->value;
    val2 = gen_type2->value;
    bool is_equals = false;
    switch (gen_type1->type) 
    {
        case GEN_TYPE_STR:
            is_equals = strcmp(val1->s_val, val2->s_val) == 0;
        case GEN_TYPE_INT:
            is_equals = *(val1->i_val) == *(val2->i_val);
        case GEN_TYPE_LONG:
            is_equals = *(val1->l_val) == *(val2->l_val);
        case GEN_TYPE_FLOAT:
            is_equals = *(val1->f_val) == *(val2->f_val);
        case GEN_TYPE_DOUBLE:
            is_equals = *(val1->d_val) == *(val2->d_val);
        case GEN_TYPE_TABLE:
            {
                s_out("todo");
                break;
            }
        case GEN_TYPE_LIST:
            {
                s_out("todo");
                break;
            }
    }

    return is_equals;
}




