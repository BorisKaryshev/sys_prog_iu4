#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    uint32_t sleep_duration;
} options_t;

typedef enum {
    OK,
    ERROR
} return_code_t_e;

return_code_t_e run(void);

#ifdef __cplusplus
}
#endif
