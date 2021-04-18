sudo gcc \
    main.c \
    string_builder.c \
    number_util.c\
    common_util.c\
    generic_type.c\
    generic_table.c\
    generic_list.c\
    json_serializer.c\
    -o\
    test\
    -lm # 連接數學庫(math.h)
./test