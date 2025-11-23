#include <gtest/gtest.h>

#include <stack.h>

TEST(example, example) {
    ASSERT_TRUE(false);
}
//
// // Простой помощник для сравнения raw-буферов
// static void expect_buffer_equal(const char* a, const char* b, uint64_t n) {
//     EXPECT_EQ(0, memcmp(a, b, (size_t)n));
// }
//
// TEST(CustomStack_Create, Default) {
//     custom_stack_t s = create_stack();
//     // начальный стек должен быть пустым
//     EXPECT_EQ((uint64_t)0, s.size);
//     // допустимо, что реализовано с ненулевой начальной ёмкостью
//     EXPECT_GE(s.capacity, (uint64_t)0);
//     // при явном выделении data обычно != nullptr
//     EXPECT_TRUE(s.capacity == 0 || s.data != nullptr);
//     free_stack(&s);
// }
//
// TEST(CustomStack_Create, WithCapacity) {
//     const uint64_t req = 64;
//     custom_stack_t s = create_stack_with_capacity(req);
//     EXPECT_GE(s.capacity, req);
//     EXPECT_EQ((uint64_t)0, s.size);
//     EXPECT_TRUE(s.capacity == 0 || s.data != nullptr);
//     free_stack(&s);
// }
//
// TEST(CustomStack_PushPop, BasicPushPop) {
//     custom_stack_t s = create_stack();
//     const char msg[] = "hello";
//     const uint64_t n = sizeof(msg) - 1; // без нуль-терминатора
//
//     push_stack(&s, (char*)msg, n);
//     EXPECT_EQ(n, s.size);
//     // прочитаем обратно
//     std::vector<char> out(n);
//     pop_stack(&s, out.data(), n);
//     EXPECT_EQ((uint64_t)0, s.size);
//     expect_buffer_equal(out.data(), msg, n);
//
//     free_stack(&s);
// }
//
// TEST(CustomStack_PushPop, MultipleBlocksLIFO) {
//     custom_stack_t s = create_stack();
//     const char a[] = "12";
//     const char b[] = "345";
//     push_stack(&s, (char*)a, 2);   // теперь стек: [1 2]
//     push_stack(&s, (char*)b, 3);   // теперь стек: [1 2 3 4 5] где верх — конец блока b
//
//     // вытащим блок b (последний)
//     std::vector<char> out1(3);
//     pop_stack(&s, out1.data(), 3);
//     expect_buffer_equal(out1.data(), b, 3);
//     EXPECT_EQ((uint64_t)2, s.size);
//
//     // затем блок a
//     std::vector<char> out2(2);
//     pop_stack(&s, out2.data(), 2);
//     expect_buffer_equal(out2.data(), a, 2);
//     EXPECT_EQ((uint64_t)0, s.size);
//
//     free_stack(&s);
// }
//
// TEST(CustomStack_Resize, GrowWhenNeeded) {
//     // запросим маленькую начальную ёмкость чтобы гарантировать расширение
//     custom_stack_t s = create_stack_with_capacity(4);
//     const uint64_t initial_capacity = s.capacity;
//
//     // создадим буфер больший, чем начальная ёмкость
//     std::vector<char> big((size_t)initial_capacity + 20, 'x');
//     push_stack(&s, big.data(), (uint64_t)big.size());
//     // емкость должна быть не меньше размера
//     EXPECT_GE(s.capacity, s.size);
//     EXPECT_GE(s.size, (uint64_t)big.size());
//
//     // прочитаем и сверим
//     std::vector<char> out(big.size());
//     pop_stack(&s, out.data(), (uint64_t)big.size());
//     EXPECT_EQ((uint64_t)0, s.size);
//     expect_buffer_equal(out.data(), big.data(), (uint64_t)big.size());
//
//     free_stack(&s);
// }
//
// TEST(CustomStack_Free, AfterFreeFieldsReset) {
//     custom_stack_t s = create_stack();
//     const char msg[] = "hi";
//     push_stack(&s, (char*)msg, 2);
//
//     free_stack(&s);
//     // Ожидаем, что ресурс освобожден и поля сброшены (реализация может отличаться,
//     // но такое поведение принято)
//     EXPECT_TRUE(s.data == nullptr || s.capacity == 0);
//     EXPECT_EQ((uint64_t)0, s.size);
// }
