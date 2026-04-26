#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

extern const uint64_t STRING_T_MAX_SIZE;

typedef struct {
    char* data;
    size_t size;
} string_t;

string_t string_create(const char* data);
uint64_t string_required_buffer_size(string_t str);
void string_serialize_to_buffer(string_t string, char* buffer);
void string_free(string_t* str);
string_t string_decerealize(const char* buffer);

void hexdump(const uint8_t* buf, size_t len);

#ifdef __cplusplus
}
#endif
