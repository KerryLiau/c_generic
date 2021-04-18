#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "../include/string_builder.h"
#include "../include/common_util.h"
#include "../include/number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
struct StringBuilder_Private
{
    int size;
    char *value;
};

static void _CheckSpace(StringBuilder* sb, int len)
{
    int cur_len = strlen(sb->priv->value);
    int max = sb->priv->size;
    if (cur_len + len >= max)
    {
        char *old_val = sb->priv->value;
        int new_max = NumberUtil_NextPowerOf_2(cur_len + len);
        char *new_val = (char*) calloc(new_max, sizeof(char));
        if (!new_val)
        {
            s_out("New StringBuilder value malloc failed");
        }
        strcpy(new_val, old_val);
        free(old_val);
        old_val = NULL;
        sb->priv->value = new_val;
        sb->priv->size = new_max;
    }
}

// ================================================================================
// Public properties
// ================================================================================
StringBuilder* New_StringBuilder(void)
{
    StringBuilder *sb = (StringBuilder*) malloc(sizeof(StringBuilder));
    StringBuilder_Private *priv = (StringBuilder_Private*) malloc(sizeof(StringBuilder_Private));
    sb->priv = priv;
    priv->size = 10;
    priv->value = (char*) calloc(sb->priv->size, sizeof(char*));
    return sb;
}

void Delete_StringBuilder(StringBuilder **p_sb)
{
    StringBuilder *sb = *p_sb;
    free(sb->priv->value);
    free(sb->priv);
    free(sb);
    *p_sb = NULL;
}

void StringBuilder_AppendString(StringBuilder *sb, char *str)
{
    _CheckSpace(sb, strlen(str));
    strcat(sb->priv->value, str);
}

void StringBuilder_AppendConstString(StringBuilder *sb, const char *str) 
{
    _CheckSpace(sb, strlen(str));
    strcat(sb->priv->value, str);
}

void StringBuilder_AppendInt(StringBuilder *sb, int i)
{
    char str[17];
    sprintf(str, "%d", i);
    _CheckSpace(sb, strlen(str));
    strcat(sb->priv->value, str);
}

void StringBuilder_AppendLong(StringBuilder *sb, long l)
{
    char str[20];
    sprintf(str, "%ld", l);
    _CheckSpace(sb, strlen(str));
    strcat(sb->priv->value, str);
}

void StringBuilder_AppendFloat(StringBuilder *sb, float f)
{
    char str[24];
    sprintf(str, "%f", f);
    _CheckSpace(sb, strlen(str));
    strcat(sb->priv->value, str);
}
void StringBuilder_AppendDouble(StringBuilder *sb, double d)
{
    char str[24];
    sprintf(str, "%f", d);
    _CheckSpace(sb, strlen(str));
    strcat(sb->priv->value, str);
}

void StringBuilder_Clear(StringBuilder *builder)
{
    int new_size = 16;
    char *new_val = (char*) calloc(new_size, sizeof(char));
    free(builder->priv->value);
    builder->priv->size = new_size;
    builder->priv->value = new_val;
}

char* StringBuilder_Value(StringBuilder *builder)
{
    int len = strlen(builder->priv->value) + 1;
    char *str = (char*) calloc(len, sizeof(char));
    strcpy(str, builder->priv->value);
    return str;
}
