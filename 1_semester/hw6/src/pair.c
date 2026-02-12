#include "pair.h"
#include <stdlib.h>
#include <string.h>

void free_pair_of_str_content(pair_of_str_t* pair) {
    free(pair->first);
    free(pair->second);
}

void free_pair_of_str(pair_of_str_t* pair) {
    free_pair_of_str_content(pair);
    free(pair);
}

custom_string_t serialize_pair_of_str(pair_of_str_t* pair) {
    uint64_t pair_total_size = 0;
    uint64_t pair_first_size = strlen(pair->first) + 1;    // str + \0
    uint64_t pair_second_size = strlen(pair->second) + 1;  // str + \0
    pair_total_size += sizeof(uint64_t);
    pair_total_size += pair_first_size;
    pair_total_size += sizeof(uint64_t);
    pair_total_size += pair_second_size;

    char* res = malloc(pair_total_size);
    char* current_ptr = res;

    memcpy(current_ptr, (char*)(&pair_first_size), sizeof(pair_first_size));
    current_ptr += sizeof(pair_first_size);

    memcpy(current_ptr, pair->first, pair_first_size);
    current_ptr += pair_first_size;

    memcpy(current_ptr, (char*)(&pair_second_size), sizeof(pair_second_size));
    current_ptr += sizeof(pair_second_size);

    memcpy(current_ptr, pair->second, pair_second_size);

    custom_string_t result = {pair_total_size, res};
    return result;
}

pair_of_str_t deserialize_pair_of_str(char* buffer) {
    pair_of_str_t result = {NULL, NULL};

    uint64_t pair_first_size = 0;
    uint64_t pair_second_size = 0;

    memcpy((char*)(&pair_first_size), buffer, sizeof(pair_first_size));
    buffer += sizeof(pair_first_size);

    if (pair_first_size) {
        result.first = malloc(pair_first_size);
        memcpy(result.first, buffer, pair_first_size);
    }

    memcpy((char*)(&pair_second_size), buffer, sizeof(pair_second_size));
    buffer += sizeof(pair_second_size);
    if (pair_second_size) {
        result.second = malloc(pair_second_size);
        memcpy(result.second, buffer, pair_second_size);
    }

    return result;
}
