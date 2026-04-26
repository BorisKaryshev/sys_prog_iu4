#include "crypto.h"
#include "string_util.h"

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

crypto_options_t crypto_options_create(const char* path) {
    char buff[CRYPTO_KEY_SIZE + CRYPTO_IV_SIZE];
    crypto_options_t opts = {.key = malloc(CRYPTO_KEY_SIZE), .iv = malloc(CRYPTO_IV_SIZE)};

    int fd;
    if ((fd = open(path, O_RDONLY)) > 0) {
        int bytes_read = read(fd, buff, CRYPTO_KEY_SIZE + CRYPTO_IV_SIZE);
        assert(bytes_read == (int)(CRYPTO_KEY_SIZE + CRYPTO_IV_SIZE));
        memcpy(opts.key, buff, CRYPTO_KEY_SIZE);
        memcpy(opts.iv, buff + CRYPTO_KEY_SIZE, CRYPTO_IV_SIZE);
        return opts;
    }

    srand(time(NULL));
    for (uint32_t i = 0; i < CRYPTO_KEY_SIZE; i++) {
        opts.key[i] = (uint8_t)rand();
    }
    for (uint32_t i = 0; i < CRYPTO_IV_SIZE; i++) {
        opts.iv[i] = (uint8_t)rand();
    }

    fd = open(path, O_WRONLY | O_CREAT | O_EXCL);
    assert(fd > 0);
    memcpy(buff, opts.key, CRYPTO_KEY_SIZE);
    memcpy(buff + CRYPTO_KEY_SIZE, opts.iv, CRYPTO_IV_SIZE);

    int bytes_read = write(fd, buff, CRYPTO_KEY_SIZE + CRYPTO_IV_SIZE);
    assert(bytes_read == (int)(CRYPTO_KEY_SIZE + CRYPTO_IV_SIZE));
    return opts;
}

void crypto_options_free(crypto_options_t* opts) {
    free(opts->key);
    free(opts->iv);

    opts->key = NULL;
    opts->iv = NULL;
}

string_t encrypt_data(crypto_options_t opts, uint8_t* message, size_t size) {
    string_t empty_result = {.size = 0, .data = NULL};

    size_t alloc_size = EVP_CIPHER_block_size(EVP_aes_256_cbc()) * ((size / EVP_CIPHER_block_size(EVP_aes_256_cbc())) + 1);

    string_t result = {.data = malloc(alloc_size), .size = alloc_size};
    if (!result.data) {
        fprintf(stderr, "encrypt_data: failed to allocate buffer\n");
        return empty_result;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "encrypt_data: EVP_CIPHER_CTX_new failed\n");
        free(result.data);
        return empty_result;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, opts.key, opts.iv) != 1) {
        fprintf(stderr, "encrypt_data: EVP_EncryptInit_ex failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        return empty_result;
    }

    int cipher_text_length = 0;
    if (EVP_EncryptUpdate(ctx, (uint8_t*)result.data, &cipher_text_length, message, size) != 1) {
        fprintf(stderr, "encrypt_data: EVP_EncryptUpdate failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        return empty_result;
    }

    int len = 0;
    if (EVP_EncryptFinal_ex(ctx, ((uint8_t*)result.data) + cipher_text_length, &len) != 1) {
        fprintf(stderr, "encrypt_data: EVP_EncryptFinal_ex failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        return empty_result;
    }
    cipher_text_length += len;

    EVP_CIPHER_CTX_free(ctx);
    return result;
}

string_t decrypt_data(crypto_options_t opts, uint8_t* message, size_t size) {
    // Allocate buffer for ciphertext (allowing for block padding)
    string_t empty_result = {.size = 0, .data = NULL};

    string_t result = {.data = malloc(size), .size = size};
    if (!result.data) {
        fprintf(stderr, "decrypt_data: failed to allocate buffer\n");
        return empty_result;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "decrypt_data: EVP_CIPHER_CTX_new failed\n");
        free(result.data);
        return empty_result;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, opts.key, opts.iv) != 1) {
        fprintf(stderr, "decrypt_data: EVP_EncryptInit_ex failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        return empty_result;
    }

    int cipher_text_length = 0;
    if (EVP_DecryptUpdate(ctx, (uint8_t*)result.data, &cipher_text_length, message, size) != 1) {
        fprintf(stderr, "decrypt_data: EVP_EncryptUpdate failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        return empty_result;
    }

    int len = 0;
    if (EVP_DecryptFinal_ex(ctx, ((uint8_t*)result.data) + cipher_text_length, &len) != 1) {
        fprintf(stderr, "decrypt_data: EVP_EncryptFinal_ex failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        return empty_result;
    }
    cipher_text_length += len;

    EVP_CIPHER_CTX_free(ctx);
    return result;
}
