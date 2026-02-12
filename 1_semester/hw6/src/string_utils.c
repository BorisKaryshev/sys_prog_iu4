#include <string_utils.h>

#include <memory.h>
#include <stdlib.h>

custom_string_t string_read_from_buffer(const char* buffer) {
    custom_string_t result;
    memcpy((char*)(&result.size), buffer, sizeof(result.size));

    result.data = malloc(result.size);
    memcpy(result.data, buffer + sizeof(result.size), result.size);
    return result;
}

void string_write_to_buffer(const custom_string_t* string, char* buffer) {
    memcpy(buffer, (char*)(&string->size), sizeof(string->size));
    memcpy(buffer + sizeof(string->size), string->data, string->size);
}

void string_free(custom_string_t* string) {
    free(string->data);
    string->size = 0;
}
