#include "list.h"

#include <gtest/gtest.h>

namespace {
int comparator (char* ptr, char* data) {
    auto p = reinterpret_cast<int*>(ptr);
    auto d = reinterpret_cast<int*>(data);
    return (*p == *d) ? 1 : 0;
};

void free_data(void* ptr) {
    free(ptr);
}
}

TEST(CustomListTest, PushPopSingle) {
    custom_list_t list = create_list();

    int v = 7;
    push_back_list(&list, reinterpret_cast<const char*>(&v), sizeof(v));

    int out = 0;
    pop_back_list(&list, reinterpret_cast<char*>(&out), sizeof(out));

    ASSERT_EQ(out, v);
    ASSERT_EQ(list.first_node, nullptr);
    ASSERT_EQ(list.last_node, nullptr);

    free_list(&list, free_data);
}

TEST(CustomListTest, PushPopMultiple) {
    custom_list_t list = create_list();

    int a = 1, b = 2, c = 3;
    push_back_list(&list, reinterpret_cast<const char*>(&a), sizeof(a));
    push_back_list(&list, reinterpret_cast<const char*>(&b), sizeof(b));
    push_back_list(&list, reinterpret_cast<const char*>(&c), sizeof(c));

    int x = 0;
    pop_back_list(&list, reinterpret_cast<char*>(&x), sizeof(x));
    ASSERT_EQ(x, 3);

    pop_back_list(&list, reinterpret_cast<char*>(&x), sizeof(x));
    ASSERT_EQ(x, 2);

    pop_back_list(&list, reinterpret_cast<char*>(&x), sizeof(x));
    ASSERT_EQ(x, 1);

    ASSERT_EQ(list.first_node, nullptr);
    ASSERT_EQ(list.last_node, nullptr);

    free_list(&list, free_data);
}

TEST(CustomListTest, FindMiddle) {
    custom_list_t list = create_list();

    int a = 5, b = 8, c = 9;
    push_back_list(&list, reinterpret_cast<const char*>(&a), sizeof(a));
    push_back_list(&list, reinterpret_cast<const char*>(&b), sizeof(b));
    push_back_list(&list, reinterpret_cast<const char*>(&c), sizeof(c));

    int tmp = b;
    custom_list_node_t* n = find(&list, comparator, reinterpret_cast<char*>(&tmp));
    ASSERT_NE(n, nullptr);

    int v = *reinterpret_cast<int*>(n->data);
    ASSERT_EQ(v, 8);

    free_list(&list, free_data);
}

TEST(CustomListTest, FindNotFound) {
    custom_list_t list = create_list();

    int a = 1;
    push_back_list(&list, reinterpret_cast<const char*>(&a), sizeof(a));


    int tmp = 100;
    custom_list_node_t* n = find(&list, comparator, reinterpret_cast<char*>(&tmp));
    ASSERT_EQ(n, nullptr);

    free_list(&list, free_data);
}

TEST(CustomListTest, DeleteFirst) {
    custom_list_t list = create_list();

    int a = 10, b = 20;
    push_back_list(&list, reinterpret_cast<const char*>(&a), sizeof(a));
    push_back_list(&list, reinterpret_cast<const char*>(&b), sizeof(b));

    custom_list_node_t* first = list.first_node;
    delete_node(first);
    list.first_node = list.last_node;

    free_list(&list, free_data);
}

TEST(CustomListTest, DeleteLast) {
    custom_list_t list = create_list();

    int a = 3, b = 4;
    push_back_list(&list, reinterpret_cast<const char*>(&a), sizeof(a));
    push_back_list(&list, reinterpret_cast<const char*>(&b), sizeof(b));

    custom_list_node_t* last = list.last_node;
    delete_node(last);

    free_list(&list, free_data);
}

TEST(CustomListTest, DeleteMiddle) {
    custom_list_t list = create_list();

    int a = 1, b = 2, c = 3;
    push_back_list(&list, reinterpret_cast<const char*>(&a), sizeof(a));
    push_back_list(&list, reinterpret_cast<const char*>(&b), sizeof(b));
    push_back_list(&list, reinterpret_cast<const char*>(&c), sizeof(c));

    custom_list_node_t* mid = list.first_node->next_node;
    delete_node(mid);

    ASSERT_EQ(*reinterpret_cast<int*>(list.first_node->data), 1);
    ASSERT_EQ(*reinterpret_cast<int*>(list.last_node->data), 3);

    ASSERT_EQ(list.first_node->next_node, list.last_node);
    ASSERT_EQ(list.last_node->prev_node, list.first_node);

    free_list(&list, free_data);
}

TEST(CustomListTest, FreeListWorksOnEmpty) {
    custom_list_t list = create_list();
    free_list(&list, free_data);

    ASSERT_EQ(list.first_node, nullptr);
    ASSERT_EQ(list.last_node, nullptr);
}
