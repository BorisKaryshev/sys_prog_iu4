#include <stack.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <vector>

static void eqbuf(const char* a, const char* b, uint64_t n) {
    ASSERT_EQ(0, std::memcmp(a, b, (size_t)n));
}

TEST(Stack, CreateDefault) {
    custom_stack_t s = create_stack();

    ASSERT_EQ(s.size, 0);
    ASSERT_GE(s.capacity, 0);
    ASSERT_TRUE(s.capacity == 0 || s.data != nullptr);

    free_stack(&s);
}

TEST(Stack, CreateWithCapacity) {
    custom_stack_t s = create_stack_with_capacity(64);

    ASSERT_EQ(s.capacity, 64);
    ASSERT_EQ(s.size, 0);
    ASSERT_NE(s.data, nullptr);

    free_stack(&s);
}

TEST(Stack, PushPopSingle) {
    custom_stack_t s = create_stack();
    double var = 123.321;

    push_stack(&s, reinterpret_cast<char*>(&var), sizeof(var));
    ASSERT_EQ(s.size, sizeof(var));

    double res = 0;
    pop_stack(&s, reinterpret_cast<char*>(&res), sizeof(res));
    ASSERT_EQ(s.size, 0);
    ASSERT_EQ(res, var);

    free_stack(&s);
}

TEST(Stack, PushPopMultipleBlocks) {
    custom_stack_t s = create_stack();
    int a = 12;
    double b = 3456.12;

    push_stack(&s, reinterpret_cast<char*>(&a), sizeof(a));
    push_stack(&s, reinterpret_cast<char*>(&b), sizeof(b));

    ASSERT_EQ(s.size, sizeof(a) + sizeof(b));

    double b_from_stack;
    pop_stack(&s, reinterpret_cast<char*>(&b_from_stack), sizeof(b));
    ASSERT_EQ(b_from_stack, b);
    ASSERT_EQ(s.size, sizeof(a));

    int a_from_stack;
    pop_stack(&s, reinterpret_cast<char*>(&a_from_stack), sizeof(a));
    ASSERT_EQ(a_from_stack, a);
    ASSERT_EQ(s.size, 0);
    free_stack(&s);
}

TEST(Stack, AutoGrow) {
    custom_stack_t s = create_stack_with_capacity(4);
    ASSERT_EQ(s.capacity, 4);
    uint64_t size = 0;
    for(int i = 0; i < 1000; i++) {
        int x = 10;
        push_stack(&s, reinterpret_cast<char*>(&x), sizeof(x));
        size += sizeof(x);

        ASSERT_GT(s.capacity, size);
    }

    free_stack(&s);
}


TEST(Stack, Free) {
    custom_stack_t s = create_stack();
    int data = 1234578;
    push_stack(&s, reinterpret_cast<char*>(&data), sizeof(data));
    free_stack(&s);

    ASSERT_EQ(s.data, nullptr);
    ASSERT_EQ(s.capacity, 0);
    ASSERT_EQ(s.size, 0);
}
