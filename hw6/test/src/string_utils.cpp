#include "string_utils.h"
#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

using namespace std;

TEST(StringUtilsTest, ReadWriteBuffer) {
    const char* data = "test data";
    uint64_t size = strlen(data);
    custom_string_t str;
    str.size = size;
    str.data = (char*)malloc(size);
    memcpy(str.data, data, size);

    char buffer[sizeof(str.size) + size];
    string_write_to_buffer(&str, buffer);

    custom_string_t read = string_read_from_buffer(buffer);

    EXPECT_EQ(read.size, size);
    EXPECT_STREQ(read.data, data);

    string_free(&read);
    free(str.data);
}
