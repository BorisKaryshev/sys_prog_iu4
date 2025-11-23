#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct custom_list_node_s;

typedef struct custom_list_node_s {
    char* data;
    struct custom_list_node_s* next_node;
    struct custom_list_node_s* prev_node;
} custom_list_node_t;

typedef struct {
    custom_list_node_t* first_node;
    custom_list_node_t* last_node;
} custom_list_t;

custom_list_t create_list(void);

void push_back(custom_list_t* list, const char* data, uint64_t size);
void pop_back(custom_list_t* list, char* data, uint64_t size);

typedef int(*list_find_eq_comparator)(char*, char*);
custom_list_node_t* find(custom_list_t* list, list_find_eq_comparator comarator, char* comparator_data);
void delete_node(custom_list_node_t* node);

void free_list(custom_list_t* list);

#ifdef __cplusplus
}
#endif
