//----------------------------------------------------------------------------
// aesbench - Copyright (c) 2024, Thierry Lelegard
// BSD 2-Clause License, see LICENSE file.
//----------------------------------------------------------------------------

#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <cstdlib>
#include <cinttypes>
#include <sys/resource.h>
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#if !defined(OPENSSL_VERSION_MAJOR) // before v3
#define OPENSSL_VERSION_MAJOR (OPENSSL_VERSION_NUMBER >> 28)
#endif

#if !defined(OPENSSL_VERSION_MINOR) // before v3
#define OPENSSL_VERSION_MINOR ((OPENSSL_VERSION_NUMBER >> 20) & 0xFF)
#endif

constexpr int64_t USECPERSEC = 1000000;  // microseconds per second
constexpr int64_t MIN_CPU_TIME = 2 * USECPERSEC;
constexpr size_t  AES_BLOCK_SIZE = 16;
constexpr size_t  BLOCK_COUNT = 1000;
constexpr size_t  INNER_LOOP_COUNT = 100000;


//----------------------------------------------------------------------------
// Get current CPU time resource usage in microseconds.
//----------------------------------------------------------------------------

int64_t cpu_time()
{
    rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) < 0) {
        perror("getrusage");
        exit(EXIT_FAILURE);
    }
    return ((int64_t)(ru.ru_utime.tv_sec) * USECPERSEC) + ru.ru_utime.tv_usec +
           ((int64_t)(ru.ru_stime.tv_sec) * USECPERSEC) + ru.ru_stime.tv_usec;
}


//----------------------------------------------------------------------------
// OpenSSL error, abort application.
//----------------------------------------------------------------------------

[[noreturn]] void fatal(const std::string& message)
{
    if (!message.empty()) {
        std::cerr << "openssl: " << message << std::endl;
    }
    ERR_print_errors_fp(stderr);
    std::exit(EXIT_FAILURE);
}


//----------------------------------------------------------------------------
// Perform one test
//----------------------------------------------------------------------------

void one_test(const EVP_CIPHER* evp)
{
    std::vector<uint8_t> key(EVP_CIPHER_key_length(evp));
    std::vector<uint8_t> iv(EVP_CIPHER_iv_length(evp), 0x47);
    std::vector<uint8_t> input(BLOCK_COUNT * AES_BLOCK_SIZE, 0xA5);
    std::vector<uint8_t> output(input.size());

    // Enforce different bytes in key.
    uint8_t byte = 0x23;
    for (auto& kbyte : key) {
        kbyte = byte++;
    }

    std::cout << "algo: " << EVP_CIPHER_name(evp) << std::endl;
    std::cout << "key-size: " << EVP_CIPHER_key_length(evp) << std::endl;
    std::cout << "iv-size: " << EVP_CIPHER_iv_length(evp) << std::endl;
    std::cout << "block-size: " << EVP_CIPHER_block_size(evp) << std::endl;
    std::cout << "data-size: " << input.size() << std::endl;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        fatal("error creating cipher context");
    }

    int output_len = 0;
    uint64_t start = 0;
    uint64_t duration = 0;
    uint64_t size = 0;

    // Encryption test.
    if (EVP_EncryptInit(ctx, evp, key.data(), iv.data()) <= 0) {
        fatal("encrypt init error");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) <= 0) {
        fatal("set no padding error");
    }

    size = 0;
    start = cpu_time();
    do {
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            if (EVP_EncryptUpdate(ctx, output.data(), &output_len, input.data(), input.size()) <= 0) {
                fatal("encrypt update error");
            }
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore EVP_EncryptFinal(), check performance only.
    std::cout << "encrypt-microsec: " << duration << std::endl;
    std::cout << "encrypt-size: " << size << std::endl;
    std::cout << "encrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;
    
    // Decryption test.
    if (EVP_DecryptInit(ctx, evp, key.data(), iv.data()) <= 0) {
        fatal("decrypt init error");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) <= 0) {
        fatal("set no padding error");
    }

    size = 0;
    start = cpu_time();
    do {
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            if (EVP_DecryptUpdate(ctx, output.data(), &output_len, input.data(), input.size()) <= 0) {
                fatal("decrypt update error");
            }
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore EVP_DecryptFinal(), check performance only.
    std::cout << "decrypt-microsec: " << duration << std::endl;
    std::cout << "decrypt-size: " << size << std::endl;
    std::cout << "decrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;

    EVP_CIPHER_CTX_free(ctx);
}


//----------------------------------------------------------------------------
// Application entry point
//----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // OpenSSL initialization.
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    std::cout << "openssl: "
#if defined(OPENSSL_FULL_VERSION_STRING) // v3
              << OpenSSL_version(OPENSSL_FULL_VERSION_STRING) << ", " << OpenSSL_version(OPENSSL_CPU_INFO)
#else
              << OpenSSL_version(OPENSSL_VERSION)
#endif
              << std::endl;

    // Run tests.
    one_test(EVP_aes_128_ecb());
    one_test(EVP_aes_256_ecb());
    one_test(EVP_aes_128_cbc());
    one_test(EVP_aes_256_cbc());
    one_test(EVP_aes_128_xts());
    one_test(EVP_aes_256_xts());
    one_test(EVP_aes_128_gcm());
    one_test(EVP_aes_256_gcm());

    // OpenSSL cleanup.
    EVP_cleanup();
    ERR_free_strings();
    return EXIT_SUCCESS;
}
