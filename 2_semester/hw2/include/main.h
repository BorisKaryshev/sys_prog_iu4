#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    const char* input_pipe_path;
    const char* output_pipe_path;
} options_t;

typedef enum {
    OK,
    ERROR
} return_code_t_e;

return_code_t_e run(options_t);

#ifdef __cplusplus
}
#endif
