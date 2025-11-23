#include <list.h>

#include <memory.h>
#include <stdlib.h>

custom_list_t create_list(void) {
    custom_list_t res;
    res.first_node = NULL;
    res.last_node = NULL;

    return res;
}

void push_back(custom_list_t* list, const char* data, uint64_t size) {
    char* new_data = malloc(size);
    memcpy(new_data, data, size);
    custom_list_node_t* new_node = malloc(sizeof(custom_list_node_t));
    new_node->data = new_data;
    new_node->prev_node = list->last_node;
    new_node->next_node = NULL;

    if(list->last_node) {
        list->last_node->next_node = new_node;
    }
    list->last_node = new_node;

    if(list->first_node == NULL) {
        list->first_node = list->last_node;
    }
}

void pop_back(custom_list_t* list, char* data, uint64_t size) {
    custom_list_node_t* node = list->last_node;
    if(node == NULL) {
        return;
    }
    memcpy(data, node->data, size);


    if(list->first_node == list->last_node) {
        list->first_node = NULL;
        list->last_node = NULL;
    } else {
        list->last_node = node->prev_node;
    }
    delete_node(node);
}

custom_list_node_t* find(custom_list_t* list, list_find_eq_comparator comarator, char* data) {
    for(custom_list_node_t* node = list->first_node; node != NULL; node = node->next_node) {
        if(comarator(node->data, data)) {
            return node;
        }
    }
    return NULL;
}

void delete_node(custom_list_node_t* node) {
    if(node == NULL) {
        return;
    }

    custom_list_node_t* prev_node = node->prev_node;
    custom_list_node_t* next_node = node->next_node;

    if(prev_node) {
        prev_node->next_node = next_node;
    }
    if(next_node) {
        next_node->prev_node = prev_node;
    }

    free(node->data);
    free(node);
}

void free_list(custom_list_t* list) {
    for(custom_list_node_t* node = list->first_node; node != NULL;) {
        free(node->data);
        custom_list_node_t* tmp = node->next_node;
        free(node);
        node = tmp;
    }
}
