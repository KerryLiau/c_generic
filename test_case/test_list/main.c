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

void List_Basic_Test()
{
    s_out("\n\nBegin list basic test");
    GenericList *list = New_GenericList();
    for (int i = 0; i < 5; i++)
    {
        GenericList_Add(list, i);
    }
    if (GenericList_IsEmpty(list)) return;
    
    int size = GenericList_Size(list);
    s_out_f("list is not empty, and has %d elements", size);
    for (int i = 0; i < size; i++)
    {
        GenericType *element = GenericList_At(list, i);
        s_out_f("the index %d element is %d", i, *GenericType_GetInt(element));
    }

    for (int i = 0; i < 3; i++)
    {
        if (!GenericList_DeleteAt(list, i))
        {
            s_out_f("delete failed when tring delete at index %d", i);
        }
        else 
        {
            s_out_f("delete index %d success", i);
        }
    }
    s_out("let's see how list is look like now");
    for (int i = 0; i < GenericList_Size(list); i++)
    {
        GenericType *element = GenericList_At(list, i);
        s_out_f("the index %d element is %d", i, *GenericType_GetInt(element));
    }
    
    Delete_GenericList(&list);
}

int main(int argc, char **argv)
{
    List_Basic_Test();
}