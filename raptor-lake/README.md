# Comparing OS and libraries on Intel Raptor Lake CPU

The directory contains results for `aesbench` on the Intel Raptor Lake (i7-13700H) chip
with various operating systems and different versions of the cryptographic libraries.

On Windows, a similar test is done using the BCrypt library instead of OpenSSL. BCrypt
is the native Microsoft cryptographic library. It is available on all Windows platforms,
for user mode or kernel mode.

## Test environment

The test environment is one single Lenovo laptop with the following configurations:

- Host Windows 24H2 and OpenSSL 3.3.2 (installed from https://slproweb.com/).
- VM Linux Ubuntu 24.10 and OpenSSL 3.3.1 (installed with Ubuntu).
- VM Linux Ubuntu 24.10 and OpenSSL 3.4.1 (recompiled from sources).

The hypervisor is VirtualBox 7.1.6.

## Conclusions

- All tests in ECB an CBC mode are equivalent. The differences are marginal.
- Using OpenSSL, the results are equivalent. The differences between operating systems,
  host or virtual machine, compilers, are marginal.
- In GCM mode, BCrypt is 50% faster than OpenSSL.
- In XTS mode, BCrypt is 80% faster than OpenSSL.

It seems that Microsoft did an excellent optimization job on the two modes which are
actually used in the real world: XTS for disk encryption, GCM for transmission (TLS 1.3).
