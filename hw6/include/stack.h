#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* data;
    uint64_t capacity;
    uint64_t size;
} custom_stack_t;

custom_stack_t create_stack(void);
custom_stack_t create_stack_with_capacity(uint64_t starting_capacity);
void push_stack(custom_stack_t* stack, char* data, uint64_t data_size);
void pop_stack(custom_stack_t* stack, char* data, uint64_t data_size);
int is_stack_empty(custom_stack_t* stack);

void free_stack(custom_stack_t* stack);

#ifdef __cplusplus
}
#endif
