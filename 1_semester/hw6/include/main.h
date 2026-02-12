#pragma once

#include <string_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DEFAULT_OPERATION,
    ADD_PASSWORD,
    REMOVE_PASSWORD,
    LIST_PASSWORDS,
    FIND_BY_KEY
} operation_t_e;

typedef enum {
    OK,
    OTHER_ERROR
} return_code_t_e;

typedef enum {
    LIST,
    STACK
} container_to_use_t_e;

typedef struct {
    operation_t_e command;
    container_to_use_t_e container_to_use;
    const char* path_to_file;
    const char* key;
    const char* value;
} pass_manager_options_t;

typedef struct {
    custom_string_t key;
    custom_string_t value;
} key_value_pair_s;

return_code_t_e run(pass_manager_options_t options);

#ifdef __cplusplus
}
#endif
