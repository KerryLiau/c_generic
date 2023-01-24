gcc `
    main.c `
    src/string_builder.c `
    src/number_util.c `
    src/common_util.c `
    src/generic_type.c `
    src/generic_table.c `
    src/generic_list.c `
    src/json_serializer.c `
    -o `
    test `
    -lm # 連接數學庫(math.h)
./test