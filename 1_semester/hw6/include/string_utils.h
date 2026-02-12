#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t size;
    char* data;
} custom_string_t;

custom_string_t string_read_from_buffer(const char* buffer);
void string_write_to_buffer(const custom_string_t* string, char* buffer);
void string_free(custom_string_t* string);

#ifdef __cplusplus
}
#endif
