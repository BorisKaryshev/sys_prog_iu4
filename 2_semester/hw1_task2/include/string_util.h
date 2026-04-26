#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern const uint64_t STRING_T_MAX_SIZE;

typedef struct {
    char* data;
} string_t;

string_t string_create(const char* data);
uint64_t string_required_buffer_size(string_t str);
void string_serialize_to_buffer(string_t string, char* buffer);
void string_free(string_t* str);
string_t string_decerealize(const char* buffer);

#ifdef __cplusplus
}
#endif
