#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/json_serializer.h"
#include "../../include/generic_list.h"
#include "../../include/generic_table.h"
#include "../../include/generic_type.h"
#include "../../include/string_builder.h"
#include "../../include/common_util.h"

void Test_GenericType_Equals()
{
    s_out("\n\nBegin GenericType_Equals test");
    GenericType *obj1 = New_GenericType(5);
    if (GenericType_Equals(obj1, obj1))
    {
        s_out("obj1 is equals it self");
    }

    GenericType *obj2 = New_GenericType(5);
    if (GenericType_Equals(obj1, obj2))
    {
        s_out("obj1 is equals obj2");
    }

    GenericType *obj3 = New_GenericType(6);
    if (!GenericType_Equals(obj1, obj3))
    {
        s_out("obj1 is not equals obj3");
    }

    if (!GenericType_Equals(obj1, NULL))
    {
        s_out("obj1 is not equals null");
    }

    if (!GenericType_Equals(NULL, NULL))
    {
        s_out("null is not equals null");
    }
}

int main(int argc, char **argv)
{
    Test_GenericType_Equals();
}