#include <assert.h>
#include <memory.h>
#include <stack.h>
#include <stddef.h>
#include <stdlib.h>

custom_stack_t create_stack(void) {
    return create_stack_with_capacity(0);
}

int is_stack_empty(const custom_stack_t* stack) {
    return (stack->data == NULL) || (stack->size == 0);
}

custom_stack_t create_stack_with_capacity(uint64_t starting_capacity) {
    custom_stack_t result;
    if (starting_capacity > 0) {
        result.data = malloc(starting_capacity);
    } else {
        result.data = NULL;
    }
    result.capacity = starting_capacity;
    result.size = 0;
    return result;
}

void increase_stack_size(custom_stack_t* stack, uint64_t new_capacity) {
    assert(new_capacity > stack->capacity);

    char* new_data = malloc(new_capacity);
    if (stack->data) {
        memcpy(new_data, stack->data, stack->capacity);
        free(stack->data);
    }
    stack->data = new_data;
    stack->capacity = new_capacity;
}

void push_stack(custom_stack_t* stack, const char* data, uint64_t data_size) {
    if (stack->data == NULL) {
        increase_stack_size(stack, data_size * 10);
    }

    if (stack->capacity <= stack->size + data_size) {
        uint64_t new_capacity = stack->capacity * 2;
        increase_stack_size(stack, new_capacity);
    }
    memcpy(stack->data + stack->size, data, data_size);
    stack->size += data_size;
}

void pop_stack(custom_stack_t* stack, char* data, uint64_t data_size) {
    memcpy(data, stack->data + stack->size - data_size, data_size);
    stack->size -= data_size;
}

void free_stack(custom_stack_t* stack, free_stack_data free_data) {
    free_data(stack->data);
    stack->data = NULL;
    stack->capacity = 0;
    stack->size = 0;
}
