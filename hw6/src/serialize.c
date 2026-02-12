#include "serialize.h"
#include <memory.h>
#include <stdlib.h>
#include "stack.h"
#include "string_utils.h"

#include "list.h"
#include "serialize.h"
#include "stack.h"
#include "string_utils.h"

// custom_string_t serialize_stack(custom_stack_t* stack, serialize_element_fp serialize_element, uint64_t element_size) {
//     uint64_t serialized_values_capacity = 10;
//     uint64_t n_of_elements = 0;
//     custom_string_t* serialized_values = malloc(serialized_values_capacity * sizeof(custom_string_t));
//
//     // Temporarily store popped elements to preserve original stack order
//     custom_stack_t tmp_stack = create_stack();
//
//     // Pop from original stack into tmp_stack while serializing
//     while (!is_stack_empty(stack)) {
//         char* data = malloc(element_size);
//         pop_stack(stack, data, element_size);
//         serialized_values[n_of_elements] = serialize_element(data);
//         push_stack(&tmp_stack, data, element_size);
//         free(data);
//
//         n_of_elements++;
//
//         if (n_of_elements == serialized_values_capacity) {
//             serialized_values_capacity *= 2;
//             custom_string_t* tmp = malloc(serialized_values_capacity * sizeof(custom_string_t));
//             memcpy(tmp, serialized_values, n_of_elements * sizeof(custom_string_t));
//             free_string_array(serialized_values, n_of_elements);
//             free(serialized_values);
//             serialized_values = tmp;
//         }
//     }
//
//     // Restore original stack from tmp_stack
//     while (!is_stack_empty(&tmp_stack)) {
//         char* data = malloc(element_size);
//         pop_stack(&tmp_stack, data, element_size);
//         push_stack(stack, data, element_size);
//         free(data);
//     }
//
//     custom_string_t res = serialize_element(serialized_values, n_of_elements);
//     free(serialized_values);
//     return res;
// }
//
// custom_string_t serialize_list(custom_stack_t* stack, serialize_element_fp serialize_element, uint64_t element_size) {
//     uint64_t serialized_values_capacity = 10;
//     uint64_t n_of_elements = 0;
//     custom_string_t* serialized_values = malloc(serialized_values_capacity * sizeof(custom_string_t));
//
//     // Temporarily store popped elements to preserve original stack order
//     custom_stack_t tmp_stack = create_stack();
//
//     // Pop from original stack into tmp_stack while serializing
//     while (!is_stack_empty(stack)) {
//         char* data = malloc(element_size);
//         pop_stack(stack, data, element_size);
//         serialized_values[n_of_elements] = serialize_element(data);
//         push_stack(&tmp_stack, data, element_size);
//         free(data);
//
//         n_of_elements++;
//
//         if (n_of_elements == serialized_values_capacity) {
//             serialized_values_capacity *= 2;
//             custom_string_t* tmp = malloc(serialized_values_capacity * sizeof(custom_string_t));
//             memcpy(tmp, serialized_values, n_of_elements * sizeof(custom_string_t));
//             free_string_array(serialized_values, n_of_elements);
//             free(serialized_values);
//             serialized_values = tmp;
//         }
//     }
//
//     custom_string_t res = serialize_strings(serialized_values, n_of_elements);
//     free(serialized_values);
//     return res;
// }
