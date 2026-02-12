#include "main.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*is_less)(char a, char b);
static inline int asc_cmp(char a, char b) {
    return a < b;
}
static inline int desc_cmp(char a, char b) {
    return a > b;
}
static inline void swap(char* a, char* b) {
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heapify(char* arr, uint64_t n, uint64_t i, is_less comp) {
    assert(arr && comp);
    uint64_t largest = i;
    uint64_t left = 2 * i + 1;
    uint64_t right = 2 * i + 2;
    if (left < n && comp(arr[largest], arr[left])) largest = left;
    if (right < n && comp(arr[largest], arr[right])) largest = right;
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest, comp);
    }
}

void sort(char* arr, uint64_t n_of_elements, is_less comparator) {
    if (!arr || n_of_elements <= 1) return;
    for (int64_t i = (int64_t)(n_of_elements / 2) - 1; i >= 0; --i) heapify(arr, n_of_elements, (uint64_t)i, comparator);
    for (int64_t i = (int64_t)n_of_elements - 1; i >= 0; --i) {
        swap(&arr[0], &arr[(uint64_t)i]);
        heapify(arr, (uint64_t)i, 0, comparator);
    }
}

return_code_t_e run(sort_ordering_t_e order, FILE* input_file, FILE* output_file) {
    size_t capacity = 1024;
    size_t length = 0;
    char* arr = malloc(capacity);
    int ch;
    while ((ch = fgetc(input_file)) != EOF) {
        if (length == capacity) {
            capacity *= 2;
            char* tmp = realloc(arr, capacity);
            arr = tmp;
        }
        arr[length++] = (char)ch;
    }
    if (order == ASCENDING)
        sort(arr, length, asc_cmp);
    else if (order == DESCENDING)
        sort(arr, length, desc_cmp);
    else {
        free(arr);
        return OTHER_ERROR;
    }
    fwrite(arr, 1, length, output_file);
    free(arr);
    return OK;
}
