# AES Benckmarks

This project runs AES encryption and decryption tests on various CPU's using the
OpenSSL cryptographic library. [Previous tests](https://github.com/lelegard/cryptobench)
have demonstrated that OpenSSL is usually the fastest cryptographic library, due
to better specialized assembly code in critical operations.

The tested chaining modes are ECB, CBC, XTS, GCM.

The results are summarized in files [RESULTS.md](RESULTS.md) and [RESULTS.txt](RESULTS.txt).
Two tables are provided:

- Encryption and decryption bitrates (how many bits are encrypted or decrypted per
  second when the CPU core runs at full speed for that operation).
- Encrypted and decrypted bits per CPU cycle. This metrics is independent of the
  CPU frequency and demonstrates the quality of implementation as well as the
  number of pipelines which are able to process AES instructions.

Remarks:
- All tested modes, except CBC encryption, can be potentially parallelized.
- For export reasons, the Arm Cortex A53 and A72 in Raspberry Pi 3 and 4
  have no specialized AES instructions. The AES algorithm is implemented
  with a portable C code which is much slower that specialized AES instructions.
  All other cores in these tests, Intel or Arm, have AES instructions.

The files RESULTS.md and RESULTS.txt are generated using the Python script `analyze-results.py`.
