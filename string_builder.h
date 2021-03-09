#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

typedef struct StringBuilder_Private StringBuilder_Private;

typedef struct StringBuilder
{
    StringBuilder_Private *priv;
} StringBuilder;

/*
 * StringBuilder 增加字串的泛型方法
 */
#define StringBuilder_Append(sb, e) _Generic((e), \
    char*: StringBuilder_AppendString,\
    const char*: StringBuilder_AppendConstString,\
    int: StringBuilder_AppendInt,\
    long: StringBuilder_AppendLong,\
    float: StringBuilder_AppendFloat,\
    double: StringBuilder_AppendDouble)\
    (sb, e)
void StringBuilder_AppendString(StringBuilder *sb, char *str);
void StringBuilder_AppendConstString(StringBuilder *sb, const char *str);
void StringBuilder_AppendInt(StringBuilder *sb, int i);
void StringBuilder_AppendLong(StringBuilder *sb, long l);
void StringBuilder_AppendFloat(StringBuilder *sb, float f);
void StringBuilder_AppendDouble(StringBuilder *sb, double d);

// 建構子
StringBuilder* New_StringBuilder();

// 解構子
void Delete_StringBuilder();

/*
 * 清空 StringBuilder 中的字串內容
 */
void StringBuilder_Clear(StringBuilder *builder);

/*
 * 取得 StringBuilder 中的字串內容複製品
 */
char* StringBuilder_Value(StringBuilder *builder);

#endif