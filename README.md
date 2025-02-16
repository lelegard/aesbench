# AES Benchmarks

This project runs AES encryption and decryption tests on various CPU's using the
OpenSSL cryptographic library. [Previous tests](https://github.com/lelegard/cryptobench)
have demonstrated that OpenSSL is usually the fastest cryptographic library, due
to better specialized assembly code in critical operations.

The tested key sizes are 128 and 256 bits.

The tested chaining modes are ECB, CBC, XTS, GCM.

## Performance results

The performances are evaluated on pure data encryption or decryption, using the
same AES key. The key scheduling is executed once only, before starting the
performance evaluation.

The results are summarized in file [RESULTS.txt](RESULTS.txt).
It is generated using the Python script `analyze-results.py`.

Two tables are provided:

- Encryption and decryption bitrates (how many bits are encrypted or decrypted per
  second when the CPU core runs at full speed for that operation).

- Encrypted and decrypted bits per CPU cycle. This metrics is independent of the
  CPU frequency and demonstrates the quality of implementation as well as the
  number of pipelines which are able to process AES instructions.

In each table, the ranking of each CPU in the line is added between brackets.

## Hardware acceleration

Most CPU cores have specialized AES instructions which speed up the key scheduling,
the encryption and the decryption. In the Arm architecture, the AES instructions
belong to the "Neon" SIMD instruction set.

Depending on the CPU architecture and implementation, AES processing is typically
15 to 20 times faster with specialized AES instructions.

For export reasons, the Arm Cortex A53 and A72 in Raspberry Pi 3 and 4 have no
specialized AES instructions. The AES algorithm is implemented with a portable C code
which is much slower. All other cores in these tests, Intel or Arm, have AES instructions.

## Parallelism

Most tested modes can be potentially parallelized. Successive 16-byte data blocks can
be encrypted or decrypted in parallel. The only exception is CBC encryption, where
each block is encrypted using the previous encrypted block (CBC decryption can be
parallelized).

Parallelism is not explicit in the OpenSSL code. It is implicitly achieved by the CPU
core, depending on the number of pipelines which are able to process specialized AES
instructions.

The results seem to indicate the following number of pipelines for AES instructions:

- 2 pipelines: Arm Neoverse N1
- 4 pipelines: Intel i7-8565U, Arm Neoverse V1, Neoverse V2
- 6 pipelines: Intel i7-13700H, Xeon Gold 6348
- 8 pipelines: Apple M1, M3

## Windows

All main results were measured on UNIX systems (Linux or macOS). OpenSSL is also available
on Windows, with mixed results. Windows also embeds a native cryptographic library named
BCrypt with a significantly different performance profile.

The subdirectory `windows` contains the Visual Studio project files to build `aesbench`.
It also contains an equivalent program which uses the BCrypt library.

The subdirectories `raptor-lake` and `apple-m3` contain comparative results between
Linux and Windows, OpenSSL and BCrypt, on Intel and Arm CPU's respectively.
