# Results comparison

## Encryption / decryption bitrate

| AES mode            | i7-8565U<br/>(4.20 GHz) | i7-13700H<br/>(5.00 GHz) | Cortex A53<br/>(1.20 GHz) | Neoverse V1<br/>(2.60 GHz) | Apple M1<br/>(3.20 GHz) | Apple M3<br/>(4.00 GHz) |
| ------------------- | ----------------------: | -----------------------: | ------------------------: | -------------------------: | ----------------------: | ----------------------: |
| AES-128-ECB encrypt |             52.031 Gb/s |             123.062 Gb/s |                0.405 Gb/s |                75.014 Gb/s |            122.233 Gb/s |            173.933 Gb/s |
| AES-128-ECB decrypt |             52.234 Gb/s |             121.429 Gb/s |                0.388 Gb/s |                72.429 Gb/s |            124.435 Gb/s |            176.807 Gb/s |
| AES-256-ECB encrypt |             37.631 Gb/s |              82.693 Gb/s |                0.307 Gb/s |                55.253 Gb/s |             89.174 Gb/s |            129.742 Gb/s |
| AES-256-ECB decrypt |             37.378 Gb/s |              87.622 Gb/s |                0.297 Gb/s |                54.122 Gb/s |             89.138 Gb/s |            129.641 Gb/s |
| AES-128-CBC encrypt |             12.612 Gb/s |              19.005 Gb/s |                0.392 Gb/s |                16.494 Gb/s |             12.726 Gb/s |             18.987 Gb/s |
| AES-128-CBC decrypt |             53.440 Gb/s |             123.993 Gb/s |                0.463 Gb/s |                71.103 Gb/s |            119.029 Gb/s |            169.198 Gb/s |
| AES-256-CBC encrypt |              9.263 Gb/s |              13.926 Gb/s |                0.294 Gb/s |                11.732 Gb/s |              9.072 Gb/s |             14.189 Gb/s |
| AES-256-CBC decrypt |             38.222 Gb/s |              88.918 Gb/s |                0.336 Gb/s |                52.193 Gb/s |             59.829 Gb/s |             79.934 Gb/s |
| AES-128-XTS encrypt |             53.081 Gb/s |              96.767 Gb/s |                0.509 Gb/s |                58.843 Gb/s |             92.747 Gb/s |            128.795 Gb/s |
| AES-128-XTS decrypt |             52.981 Gb/s |              96.669 Gb/s |                0.454 Gb/s |                58.824 Gb/s |             92.729 Gb/s |            127.541 Gb/s |
| AES-256-XTS encrypt |             38.318 Gb/s |              77.677 Gb/s |                0.372 Gb/s |                45.904 Gb/s |             67.981 Gb/s |             99.917 Gb/s |
| AES-256-XTS decrypt |             37.846 Gb/s |              77.788 Gb/s |                0.330 Gb/s |                45.860 Gb/s |             67.924 Gb/s |             99.182 Gb/s |
| AES-128-GCM encrypt |             50.079 Gb/s |              70.009 Gb/s |                0.355 Gb/s |                36.401 Gb/s |             68.889 Gb/s |             67.512 Gb/s |
| AES-128-GCM decrypt |             51.110 Gb/s |              70.255 Gb/s |                0.354 Gb/s |                38.305 Gb/s |             69.252 Gb/s |             66.442 Gb/s |
| AES-256-GCM encrypt |             36.673 Gb/s |              61.030 Gb/s |                0.283 Gb/s |                31.406 Gb/s |             57.110 Gb/s |             64.237 Gb/s |
| AES-256-GCM decrypt |             37.283 Gb/s |              61.072 Gb/s |                0.283 Gb/s |                33.230 Gb/s |             57.951 Gb/s |             64.553 Gb/s |

## Encrypted / decrypted bits per processor cycle

| AES mode            | i7-8565U<br/>(4.20 GHz) | i7-13700H<br/>(5.00 GHz) | Cortex A53<br/>(1.20 GHz) | Neoverse V1<br/>(2.60 GHz) | Apple M1<br/>(3.20 GHz) | Apple M3<br/>(4.00 GHz) |
| ------------------- | ----------------------: | -----------------------: | ------------------------: | -------------------------: | ----------------------: | ----------------------: |
| AES-128-ECB encrypt |               12.4 b/cy |                24.6 b/cy |                0.338 b/cy |                  28.9 b/cy |               38.2 b/cy |               43.5 b/cy |
| AES-128-ECB decrypt |               12.4 b/cy |                24.3 b/cy |                0.323 b/cy |                  27.9 b/cy |               38.9 b/cy |               44.2 b/cy |
| AES-256-ECB encrypt |               8.96 b/cy |                16.5 b/cy |                0.256 b/cy |                  21.3 b/cy |               27.9 b/cy |               32.4 b/cy |
| AES-256-ECB decrypt |               8.90 b/cy |                17.5 b/cy |                0.247 b/cy |                  20.8 b/cy |               27.9 b/cy |               32.4 b/cy |
| AES-128-CBC encrypt |               3.00 b/cy |                3.80 b/cy |                0.327 b/cy |                  6.34 b/cy |               3.98 b/cy |               4.75 b/cy |
| AES-128-CBC decrypt |               12.7 b/cy |                24.8 b/cy |                0.386 b/cy |                  27.3 b/cy |               37.2 b/cy |               42.3 b/cy |
| AES-256-CBC encrypt |               2.21 b/cy |                2.79 b/cy |                0.245 b/cy |                  4.51 b/cy |               2.83 b/cy |               3.55 b/cy |
| AES-256-CBC decrypt |               9.10 b/cy |                17.8 b/cy |                0.280 b/cy |                  20.1 b/cy |               18.7 b/cy |               20.0 b/cy |
| AES-128-XTS encrypt |               12.6 b/cy |                19.4 b/cy |                0.424 b/cy |                  22.6 b/cy |               29.0 b/cy |               32.2 b/cy |
| AES-128-XTS decrypt |               12.6 b/cy |                19.3 b/cy |                0.379 b/cy |                  22.6 b/cy |               29.0 b/cy |               31.9 b/cy |
| AES-256-XTS encrypt |               9.12 b/cy |                15.5 b/cy |                0.310 b/cy |                  17.7 b/cy |               21.2 b/cy |               25.0 b/cy |
| AES-256-XTS decrypt |               9.01 b/cy |                15.6 b/cy |                0.275 b/cy |                  17.6 b/cy |               21.2 b/cy |               24.8 b/cy |
| AES-128-GCM encrypt |               11.9 b/cy |                14.0 b/cy |                0.296 b/cy |                  14.0 b/cy |               21.5 b/cy |               16.9 b/cy |
| AES-128-GCM decrypt |               12.2 b/cy |                14.1 b/cy |                0.295 b/cy |                  14.7 b/cy |               21.6 b/cy |               16.6 b/cy |
| AES-256-GCM encrypt |               8.73 b/cy |                12.2 b/cy |                0.236 b/cy |                  12.1 b/cy |               17.8 b/cy |               16.1 b/cy |
| AES-256-GCM decrypt |               8.88 b/cy |                12.2 b/cy |                0.235 b/cy |                  12.8 b/cy |               18.1 b/cy |               16.1 b/cy |