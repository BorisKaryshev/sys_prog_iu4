#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef enum {
    ASCENDING,
    DESCENDING
} sort_ordering_t_e;

typedef enum {
    OK,
    OTHER_ERROR
} return_code_t_e;

typedef enum {
    LIST,
    STACK
} container_to_use_t_e;

return_code_t_e run(sort_ordering_t_e, FILE* input_file, FILE* output_file);

#ifdef __cplusplus
}
#endif
