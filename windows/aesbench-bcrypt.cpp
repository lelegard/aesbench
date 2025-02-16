//----------------------------------------------------------------------------
// aesbench - Copyright (c) 2024, Thierry Lelegard
// BSD 2-Clause License, see LICENSE file.
//----------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cinttypes>
#include <Windows.h>

constexpr int64_t USECPERSEC = 1000000;  // microseconds per second
constexpr int64_t MIN_CPU_TIME = 2 * USECPERSEC;
constexpr size_t  AES_BLOCK_SIZE = 16;
constexpr size_t  GCM_NONCE_SIZE = 12;
constexpr size_t  XTS_MESSAGE_SIZE = 4096;
constexpr size_t  XTS_IV_SIZE = 8;  // ????? required by BCrypt
constexpr size_t  BLOCK_COUNT = 1000;
constexpr size_t  INNER_LOOP_COUNT = 100000;


//----------------------------------------------------------------------------
// Get current CPU time resource usage in microseconds.
//----------------------------------------------------------------------------

int64_t cpu_time()
{
    FILETIME creation_time, exit_time, kernel_time, user_time;
    if (GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time) == 0) {
        std::cerr << "GetProcessTimes error 0x" << std::hex << GetLastError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // A FILETIME is a 64-bit value in 100-nanosecond units (10 microsecond).
    const int64_t ktime = (int64_t(kernel_time.dwHighDateTime) << 32) | kernel_time.dwLowDateTime;
    const int64_t utime = (int64_t(user_time.dwHighDateTime) << 32) | user_time.dwLowDateTime;
    return (ktime + utime) / 10;
}


//----------------------------------------------------------------------------
// BCrypt error checking, abort application on error.
//----------------------------------------------------------------------------

void check(const char* function, NTSTATUS status)
{
    if (!BCRYPT_SUCCESS(status)) {
        std::string message(1024, 0);
        DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, status, 0, &message[0], DWORD(message.size()), nullptr);
        message.resize(std::min<size_t>(message.size(), length));
        std::cerr << "BCrypt error 0x" << std::hex << status << " in " << function << ": " << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}


//----------------------------------------------------------------------------
// Perform one test, using a generic chaining mode.
//----------------------------------------------------------------------------

void one_test_generic(const char* algo_name, size_t key_bits, size_t iv_size, LPCWSTR chain_mode)
{
    // Open algorithm provider (AES).
    BCRYPT_ALG_HANDLE algo = nullptr;
    check("BCryptOpenAlgorithmProvider",
        BCryptOpenAlgorithmProvider(&algo, BCRYPT_AES_ALGORITHM, nullptr, 0));

    // Set chaining mode.
    check("BCryptSetProperty(BCRYPT_CHAINING_MODE)",
        BCryptSetProperty(algo, BCRYPT_CHAINING_MODE, PUCHAR(chain_mode), sizeof(chain_mode), 0));

    // Get the size of the "key object" for that algo.
    DWORD objlength = 0;
    ULONG retsize = 0;
    check("BCryptGetProperty(BCRYPT_OBJECT_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_OBJECT_LENGTH, PUCHAR(&objlength), sizeof(objlength), &retsize, 0));

    // Get the block size. Just a test, should be AES block size.
    DWORD block_size = 0;
    check("BCryptGetProperty(BCRYPT_BLOCK_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_BLOCK_LENGTH, PUCHAR(&block_size), sizeof(block_size), &retsize, 0));

    std::vector<uint8_t> key(key_bits / 8, 0);
    std::vector<uint8_t> key_object(objlength, 0);
    std::vector<uint8_t> iv(iv_size, 0x47);
    std::vector<uint8_t> input(BLOCK_COUNT * block_size, 0xA5);
    std::vector<uint8_t> output(input.size() + block_size, 0);

    // Enforce different bytes in key.
    uint8_t byte = 0x23;
    for (auto& kbyte : key) {
        kbyte = byte++;
    }

    // Build a key data blob (header, followed by key).
    std::vector<uint8_t> key_data(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + key.size());
    BCRYPT_KEY_DATA_BLOB_HEADER* header = reinterpret_cast<BCRYPT_KEY_DATA_BLOB_HEADER*>(key_data.data());
    header->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
    header->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
    header->cbKeyData = ULONG(key.size());
    memcpy(key_data.data() + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), key.data(), key.size());

    // Create a new key handle.
    BCRYPT_KEY_HANDLE hkey = nullptr;
    check("BCryptImportKey",
          BCryptImportKey(algo, nullptr, BCRYPT_KEY_DATA_BLOB, &hkey,
                          key_object.data(), ULONG(key_object.size()),
                          PUCHAR(key_data.data()), ULONG(key_data.size()), 0));

    std::cout << "algo: " << algo_name << std::endl;
    std::cout << "key-size: " << key.size() << std::endl;
    std::cout << "iv-size: " << iv.size() << std::endl;
    std::cout << "block-size: " << block_size << std::endl;
    std::cout << "data-size: " << input.size() << std::endl;

    int output_len = 0;
    uint64_t start = 0;
    uint64_t duration = 0;
    uint64_t size = 0;

    // Encryption test.
    size = 0;
    start = cpu_time();
    do {
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            check("BCryptEncrypt",
                  BCryptEncrypt(hkey, PUCHAR(input.data()), ULONG(input.size()), nullptr,
                                PUCHAR(iv.data()), ULONG(iv.size()),
                                PUCHAR(output.data()), ULONG(output.size()),
                                &retsize, 0));
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore last block with BCRYPT_BLOCK_PADDING, check performance only.
    std::cout << "encrypt-microsec: " << duration << std::endl;
    std::cout << "encrypt-size: " << size << std::endl;
    std::cout << "encrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;
    
    // Decryption test.
    size = 0;
    start = cpu_time();
    do {
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            check("BCryptDecrypt",
                  BCryptDecrypt(hkey, PUCHAR(input.data()), ULONG(input.size()), nullptr,
                                PUCHAR(iv.data()), ULONG(iv.size()),
                                PUCHAR(output.data()), ULONG(output.size()),
                                &retsize, 0));
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore last block with BCRYPT_BLOCK_PADDING, check performance only.
    std::cout << "decrypt-microsec: " << duration << std::endl;
    std::cout << "decrypt-size: " << size << std::endl;
    std::cout << "decrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;

    check("BCryptDestroyKey", BCryptDestroyKey(hkey));
    check("BCryptCloseAlgorithmProvider", BCryptCloseAlgorithmProvider(algo, 0));
}


//----------------------------------------------------------------------------
// Perform one test, using a XTS chaining mode.
//----------------------------------------------------------------------------

void one_test_xts(const char* algo_name, size_t key_bits)
{
    // Open algorithm provider (AES).
    BCRYPT_ALG_HANDLE algo = nullptr;
    check("BCryptOpenAlgorithmProvider",
        BCryptOpenAlgorithmProvider(&algo, BCRYPT_XTS_AES_ALGORITHM, nullptr, 0));

    // Get the size of the "key object" for that algo.
    DWORD objlength = 0;
    ULONG retsize = 0;
    check("BCryptGetProperty(BCRYPT_OBJECT_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_OBJECT_LENGTH, PUCHAR(&objlength), sizeof(objlength), &retsize, 0));

    // Get the block size. Just a test, should be AES block size.
    DWORD block_size = 0;
    check("BCryptGetProperty(BCRYPT_BLOCK_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_BLOCK_LENGTH, PUCHAR(&block_size), sizeof(block_size), &retsize, 0));

    // Get the possible key lengths.
    BCRYPT_KEY_LENGTHS_STRUCT klengths {0, 0, 0};
    check("BCryptGetProperty(BCRYPT_KEY_LENGTHS)",
        BCryptGetProperty(algo, BCRYPT_KEY_LENGTHS, PUCHAR(&klengths), sizeof(klengths), &retsize, 0));

    std::vector<uint8_t> key(2 * (key_bits / 8), 0);  // XTS requires 2 keys: one data AES key and one tweak AES key
    std::vector<uint8_t> key_object(objlength, 0);
    std::vector<uint8_t> iv(XTS_IV_SIZE, 0x47);
    std::vector<uint8_t> input(XTS_MESSAGE_SIZE, 0xA5);
    std::vector<uint8_t> output(input.size(), 0);

    std::cout << "algo: " << algo_name << std::endl;
    std::cout << "key-size: " << key.size() << std::endl;
    std::cout << "iv-size: " << iv.size() << std::endl;
    std::cout << "block-size: " << block_size << std::endl;
    std::cout << "data-size: " << input.size() << std::endl;
    std::cout << "all-key-size: min: " << klengths.dwMinLength << ", max: " << klengths.dwMaxLength << ", incr: " << klengths.dwIncrement << std::endl;

    // Enforce different bytes in key.
    uint8_t byte = 0x23;
    for (auto& kbyte : key) {
        kbyte = byte++;
    }

    // Must set a message block length with XTS.
    DWORD message_length = DWORD(XTS_MESSAGE_SIZE);
    check("BCryptSetProperty(BCRYPT_MESSAGE_BLOCK_LENGTH)",
        BCryptSetProperty(algo, BCRYPT_MESSAGE_BLOCK_LENGTH, PUCHAR(&message_length), sizeof(message_length), 0));

    // Build a key data blob (header, followed by key).
    std::vector<uint8_t> key_data(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + key.size());
    BCRYPT_KEY_DATA_BLOB_HEADER* header = reinterpret_cast<BCRYPT_KEY_DATA_BLOB_HEADER*>(key_data.data());
    header->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
    header->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
    header->cbKeyData = ULONG(key.size());
    memcpy(key_data.data() + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), key.data(), key.size());

    // Create a new key handle.
    BCRYPT_KEY_HANDLE hkey = nullptr;
    check("BCryptImportKey",
        BCryptImportKey(algo, nullptr, BCRYPT_KEY_DATA_BLOB, &hkey,
            key_object.data(), ULONG(key_object.size()),
            PUCHAR(key_data.data()), ULONG(key_data.size()), 0));

    int output_len = 0;
    uint64_t start = 0;
    uint64_t duration = 0;
    uint64_t size = 0;

    // Encryption test.
    size = 0;
    start = cpu_time();
    do {
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            check("BCryptEncrypt",
                BCryptEncrypt(hkey, PUCHAR(input.data()), ULONG(input.size()), nullptr,
                    PUCHAR(iv.data()), ULONG(iv.size()),
                    PUCHAR(output.data()), ULONG(output.size()),
                    &retsize, 0));
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore last block with BCRYPT_BLOCK_PADDING, check performance only.
    std::cout << "encrypt-microsec: " << duration << std::endl;
    std::cout << "encrypt-size: " << size << std::endl;
    std::cout << "encrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;

    // Decryption test.
    size = 0;
    start = cpu_time();
    do {
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            check("BCryptDecrypt",
                BCryptDecrypt(hkey, PUCHAR(input.data()), ULONG(input.size()), nullptr,
                    PUCHAR(iv.data()), ULONG(iv.size()),
                    PUCHAR(output.data()), ULONG(output.size()),
                    &retsize, 0));
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore last block with BCRYPT_BLOCK_PADDING, check performance only.
    std::cout << "decrypt-microsec: " << duration << std::endl;
    std::cout << "decrypt-size: " << size << std::endl;
    std::cout << "decrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;

    check("BCryptDestroyKey", BCryptDestroyKey(hkey));
    check("BCryptCloseAlgorithmProvider", BCryptCloseAlgorithmProvider(algo, 0));
}


//----------------------------------------------------------------------------
// Perform one test, using a GCM chaining mode.
//----------------------------------------------------------------------------

void one_test_gcm(const char* algo_name, size_t key_bits)
{
    // Open algorithm provider (AES).
    BCRYPT_ALG_HANDLE algo = nullptr;
    check("BCryptOpenAlgorithmProvider",
        BCryptOpenAlgorithmProvider(&algo, BCRYPT_AES_ALGORITHM, nullptr, 0));

    // Set chaining mode.
    const LPCWSTR chain_mode = BCRYPT_CHAIN_MODE_GCM;
    check("BCryptSetProperty(BCRYPT_CHAINING_MODE)",
        BCryptSetProperty(algo, BCRYPT_CHAINING_MODE, PUCHAR(chain_mode), sizeof(chain_mode), 0));

    // Get the size of the "key object" for that algo.
    DWORD objlength = 0;
    ULONG retsize = 0;
    check("BCryptGetProperty(BCRYPT_OBJECT_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_OBJECT_LENGTH, PUCHAR(&objlength), sizeof(objlength), &retsize, 0));

    // Get the block size. Just a test, should be AES block size.
    DWORD block_size = 0;
    check("BCryptGetProperty(BCRYPT_BLOCK_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_BLOCK_LENGTH, PUCHAR(&block_size), sizeof(block_size), &retsize, 0));

    // Get the authentication tag length.
    BCRYPT_AUTH_TAG_LENGTHS_STRUCT tags {0, 0, 0};
    check("BCryptGetProperty(BCRYPT_AUTH_TAG_LENGTH)",
        BCryptGetProperty(algo, BCRYPT_AUTH_TAG_LENGTH, PUCHAR(&tags), sizeof(tags), &retsize, 0));

    std::vector<uint8_t> key(key_bits / 8, 0);
    std::vector<uint8_t> key_object(objlength, 0);
    std::vector<uint8_t> iv(block_size, 0x47);
    std::vector<uint8_t> nonce(GCM_NONCE_SIZE, 0xA4);
    std::vector<uint8_t> auth_tag(tags.dwMinLength, 0x6D);
    std::vector<uint8_t> input(BLOCK_COUNT * block_size, 0xA5);
    std::vector<uint8_t> output(input.size() + block_size, 0);

    // Enforce different bytes in key.
    uint8_t byte = 0x23;
    for (auto& kbyte : key) {
        kbyte = byte++;
    }

    // Build a key data blob (header, followed by key).
    std::vector<uint8_t> key_data(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + key.size());
    BCRYPT_KEY_DATA_BLOB_HEADER* header = reinterpret_cast<BCRYPT_KEY_DATA_BLOB_HEADER*>(key_data.data());
    header->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
    header->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
    header->cbKeyData = ULONG(key.size());
    memcpy(key_data.data() + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), key.data(), key.size());

    // Create a new key handle.
    BCRYPT_KEY_HANDLE hkey = nullptr;
    check("BCryptImportKey",
        BCryptImportKey(algo, nullptr, BCRYPT_KEY_DATA_BLOB, &hkey,
            key_object.data(), ULONG(key_object.size()),
            PUCHAR(key_data.data()), ULONG(key_data.size()), 0));

    // GCM authentication info.
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO auth_info;
    BCRYPT_INIT_AUTH_MODE_INFO(auth_info);
    std::vector<uint8_t> mac_context(tags.dwMaxLength, 0);
    auth_info.pbNonce = PUCHAR(nonce.data());
    auth_info.cbNonce = ULONG(nonce.size());
    auth_info.pbTag   = PUCHAR(auth_tag.data());
    auth_info.cbTag   = ULONG(auth_tag.size());
    auth_info.pbMacContext = PUCHAR(mac_context.data());
    auth_info.cbMacContext = ULONG(mac_context.size());

    std::cout << "algo: " << algo_name << std::endl;
    std::cout << "key-size: " << key.size() << std::endl;
    std::cout << "iv-size: " << iv.size() << std::endl;
    std::cout << "block-size: " << block_size << std::endl;
    std::cout << "data-size: " << input.size() << std::endl;
    std::cout << "auth-tag-size: min: " << tags.dwMinLength << ", max: " << tags.dwMaxLength << ", incr: " << tags.dwIncrement << std::endl;

    int output_len = 0;
    uint64_t start = 0;
    uint64_t duration = 0;
    uint64_t size = 0;

    // Encryption test.
    size = 0;
    start = cpu_time();
    do {
        auth_info.dwFlags = BCRYPT_AUTH_MODE_CHAIN_CALLS_FLAG;
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            check("BCryptEncrypt",
                BCryptEncrypt(hkey, PUCHAR(input.data()), ULONG(input.size()), &auth_info,
                    PUCHAR(iv.data()), ULONG(iv.size()),
                    PUCHAR(output.data()), ULONG(output.size()),
                    &retsize, 0));
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore last block, check performance only.
    std::cout << "encrypt-microsec: " << duration << std::endl;
    std::cout << "encrypt-size: " << size << std::endl;
    std::cout << "encrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;

    // Decryption test.
    size = 0;
    start = cpu_time();
    do {
        auth_info.dwFlags = BCRYPT_AUTH_MODE_CHAIN_CALLS_FLAG;
        for (size_t i = 0; i < INNER_LOOP_COUNT; i++) {
            check("BCryptDecrypt",
                BCryptDecrypt(hkey, PUCHAR(input.data()), ULONG(input.size()), &auth_info,
                    PUCHAR(iv.data()), ULONG(iv.size()),
                    PUCHAR(output.data()), ULONG(output.size()),
                    &retsize, 0));
            size += input.size();
        }
        duration = cpu_time() - start;
    } while (duration < MIN_CPU_TIME);
    // Ignore last block, check performance only.
    std::cout << "decrypt-microsec: " << duration << std::endl;
    std::cout << "decrypt-size: " << size << std::endl;
    std::cout << "decrypt-bitrate: " << ((USECPERSEC * 8 * size) / duration) << std::endl;

    check("BCryptDestroyKey", BCryptDestroyKey(hkey));
    check("BCryptCloseAlgorithmProvider", BCryptCloseAlgorithmProvider(algo, 0));
}


//----------------------------------------------------------------------------
// Application entry point
//----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    one_test_generic("AES-128-ECB", 128, 0, BCRYPT_CHAIN_MODE_ECB);
    one_test_generic("AES-256-ECB", 256, 0, BCRYPT_CHAIN_MODE_ECB);
    one_test_generic("AES-128-CBC", 128, AES_BLOCK_SIZE, BCRYPT_CHAIN_MODE_CBC);
    one_test_generic("AES-256-CBC", 256, AES_BLOCK_SIZE, BCRYPT_CHAIN_MODE_CBC);
    one_test_xts("AES-128-XTS", 128);
    one_test_xts("AES-256-XTS", 256);
    one_test_gcm("AES-128-GCM", 128);
    one_test_gcm("AES-256-GCM", 256);

    return EXIT_SUCCESS;
}
