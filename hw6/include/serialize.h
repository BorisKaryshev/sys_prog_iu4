#pragma once

#include "string_utils.h"


#include "stack.h"
#include "list.h"
#include "string_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void free_string_array(custom_string_t* arr, uint64_t n_of_elements);

typedef custom_string_t(*serialize_element_fp)(char* data);

custom_string_t serialize_stack(custom_stack_t* stack, serialize_element_fp serialize_element, uint64_t element_size);
custom_string_t serialize_list(custom_list_t* list, serialize_element_fp serialize_element);

#ifdef __cplusplus
}
#endif
