#include "pair.h"
#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

using namespace std;

static custom_string_t serialize_int(void* data) {
    int val = *(int*)data;
    custom_string_t s;
    s.size = sizeof(val);
    s.data = (char*)malloc(s.size);
    memcpy(s.data, &val, s.size);
    return s;
}

TEST(PairTest, SerializeDeserialize) {
    // Add tests for empty string serialization
    pair_of_str_t empty_pair;
    empty_pair.first = strdup("");
    empty_pair.second = strdup("");
    custom_string_t empty_serialized = serialize_pair_of_str(&empty_pair);
    EXPECT_EQ(empty_serialized.size, 18u); // 8+0+8+0
    uint64_t first_size = 0, second_size = 0;
    memcpy(&first_size, empty_serialized.data, sizeof(first_size));
    memcpy(&second_size, empty_serialized.data + sizeof(first_size) + first_size, sizeof(second_size));
    EXPECT_EQ(first_size, 1u);
    EXPECT_EQ(second_size, 1u);
    pair_of_str_t empty_deserialized = deserialize_pair_of_str(empty_serialized.data);
    EXPECT_STREQ(empty_deserialized.first, "");
    EXPECT_STREQ(empty_deserialized.second, "");
    free_pair_of_str_content(&empty_deserialized);
    free_pair_of_str_content(&empty_pair);
}
