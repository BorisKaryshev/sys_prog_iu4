#pragma once

#include "string_util.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

static const uint32_t CRYPTO_KEY_SIZE = 256;
static const uint32_t CRYPTO_IV_SIZE = 128;

typedef struct {
    uint8_t* key;
    uint8_t* iv;
} crypto_options_t;

string_t encrypt_data(crypto_options_t opts, uint8_t* message, size_t size);
string_t decrypt_data(crypto_options_t opts, uint8_t* message, size_t size);

crypto_options_t crypto_options_create(const char* path);
void crypto_options_free(crypto_options_t* opts);

#ifdef __cplusplus
}
#endif
