#ifndef SYS_PROG_HS6_PAIR_H
#define SYS_PROG_HS6_PAIR_H

#include "string_utils.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    char* first;
    char* second;
} pair_of_str_t;

void free_pair_of_str_content(pair_of_str_t *pair);

void free_pair_of_str(pair_of_str_t* pair);

custom_string_t serialize_pair_of_str(pair_of_str_t* pair);

pair_of_str_t deserialize_pair_of_str(char* buffer);

#ifdef __cplusplus
}
#endif
#endif
