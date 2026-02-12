#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DEFAULT_OPERATION,
    CREATE_FILE,
    DELETE_FILE
} operation_t_e;

typedef enum {
    OK,
    CREATE_FILE_EXISTS,
    DELETE_NOT_FOUND,
    UNEXPECTED_OPERATION,
    OTHER_ERROR
} return_code_t_e;

return_code_t_e run(operation_t_e operation, const char* path);

#ifdef __cplusplus
}
#endif
