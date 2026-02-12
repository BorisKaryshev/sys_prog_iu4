#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string_util.h>

const uint64_t STRING_T_MAX_SIZE = 4100;

uint64_t string_required_buffer_size(string_t str) {
    return strlen(str.data) + sizeof(uint64_t) + 1;
}

void string_serialize_to_buffer(const string_t string, char* buffer) {
    uint64_t len = strlen(string.data) + 1;
    uint64_t offset = 0;

    memcpy(buffer + offset, &len, sizeof(len));
    offset += sizeof(len);
    memcpy(buffer + offset, string.data, len);
}

void string_free(string_t* str) {
    free(str->data);
    str->data = NULL;
}
string_t string_decerealize(const char* buffer) {
    uint64_t len;
    uint64_t offset = 0;
    memcpy(&len, buffer + offset, sizeof(len));

    string_t output;
    output.data = malloc(len);

    offset += sizeof(len);
    memcpy(output.data, buffer + offset, len);
    return output;
}

string_t string_create(const char* data) {
    uint64_t len = strlen(data) + 1;
    assert((len + sizeof(len)) < STRING_T_MAX_SIZE);

    string_t output;
    output.data = calloc(len, 1);
    memcpy(output.data, data, len);
    return output;
}
