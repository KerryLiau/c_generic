sudo gcc \
    main.c \
    string_builder.c \
    number_util.c\
    hash_table.c -o \
    test\
    -lm # 連接數學庫(math.h)
./test