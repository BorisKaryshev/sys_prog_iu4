#include "string_util.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint64_t STRING_T_MAX_SIZE = 4100;

uint64_t string_required_buffer_size(string_t str) {
    return str.size + sizeof(uint64_t);
}

void string_serialize_to_buffer(const string_t string, char* buffer) {
    uint64_t len = string.size;
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
    output.size = len;

    offset += sizeof(len);
    memcpy(output.data, buffer + offset, len);
    return output;
}

string_t string_create(const char* data) {
    uint64_t len = strlen(data) + 1;
    assert((len + sizeof(len)) < STRING_T_MAX_SIZE);

    string_t output;
    output.data = calloc(len, 1);
    output.size = len;
    memcpy(output.data, data, len);
    return output;
}

void hexdump(const uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; i += 16) {
        printf("%04zx - ", i);

        /* hex */
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");

            if (j == 7) printf("-");
        }

        printf("  ");

        /* ascii */
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = buf[i + j];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
    }
}
