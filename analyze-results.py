#!/usr/bin/env python
#----------------------------------------------------------------------------
# aesbench - Copyright (c) 2024, Thierry Lelegard
# BSD 2-Clause License, see LICENSE file.
# Analyze results files and produce an analysis in markdown format.
#----------------------------------------------------------------------------

import os, sys, string, pprint
GIGA = 1000000000.0

# List of CPU cores and corresponding result files.
results = [
    {'cpu': 'i7-8565U',    'frequency': 4.20, 'file': 'intel-i7-8565U-linux-vm.txt'},
    {'cpu': 'i7-13700H',   'frequency': 5.00, 'file': 'intel-i7-13700H-linux-vm.txt'},
    {'cpu': 'Cortex A53',  'frequency': 1.20, 'file': 'arm-rpi3-cortex-a53-linux.txt'},
    {'cpu': 'Neoverse V1', 'frequency': 2.60, 'file': 'arm-graviton3-neoverse-v1-linux-vm.txt'},
    {'cpu': 'Apple M1',    'frequency': 3.20, 'file': 'arm-apple-m1-macos.txt'},
    {'cpu': 'Apple M3',    'frequency': 4.00, 'file': 'arm-apple-m3-macos.txt'}
]

# Main code: load all result files.
rootdir = os.path.dirname(os.path.abspath(sys.argv[0]))
algos = []
for res in results:
    with open(rootdir + '/results/' + res['file'], 'r') as input:
        res['data'] = {}
        algo = None
        for line in input:
            line = line.strip().split(':')
            if len(line) == 2:
                if line[0] == 'algo':
                    algo = line[1].strip()
                    if algo.startswith('id-aes'):
                        algo = 'AES-' + algo[6:]
                    if algo not in algos:
                        algos += [algo]
                    res['data'][algo] = {'encrypt': 0, 'decrypt': 0}
                elif line[0] == 'encrypt-bitrate' and algo is not None:
                    res['data'][algo]['encrypt'] = int(line[1])
                elif line[0] == 'decrypt-bitrate' and algo is not None:
                    res['data'][algo]['decrypt'] = int(line[1])

# Generate a markdown table.
# Results are either bits/second or bits/cycle.
def generate_table(gen_bitrate):
    # Compute width of first column.
    width_0 = 0
    for name in algos:
        width_0 = max(width_0, len(name))
    width_0 += len(' encrypt')
    # Compute width of each processor column.
    for res in results:
        res['header'] = '%s<br/>(%.2f GHz)' % (res['cpu'], res['frequency'])
        res['width'] = len(res['header'])
    # Output headers lines.
    print('| %*s |' % (-width_0, 'AES mode'), end='')
    for res in results:
        print(' %s |' % (res['header']), end='')
    print('')
    print('| %s |' % (width_0 * '-'), end='')
    for res in results:
        print(' %s: |' % ((res['width'] - 1) * '-'), end='')
    print('')
    # Output one line per AES operation.
    for algo in algos:
        for op in ['encrypt', 'decrypt']:
            print('| %*s |' % (-width_0, algo + ' ' + op), end='')
            for res in results:
                if algo in res['data'] and op in res['data'][algo]:
                    value = res['data'][algo][op]
                else:
                    value = 0
                if value <= 0 or (not gen_bitrate and res['frequency'] <= 0.0):
                    print(' %*s |' % (res['width'], ''), end='')
                elif gen_bitrate:
                    print(' %*.3f Gb/s |' % (res['width'] - 5, float(value) / GIGA), end='')
                else:
                    rate = float(value) / (res['frequency'] * GIGA)
                    if rate >= 10.0:
                        s = '%.1f' % rate
                    elif rate >= 1.0:
                        s = '%.2f' % rate
                    else:
                        s = '%.3f' % rate
                    print(' %*s b/cy |' % (res['width'] - 5, s), end='')
            print('')

# Generate the final markdown file.
print('# Results comparison')
print('')
print('## Encryption / decryption bitrate')
print('')
generate_table(True)
print('')
print('## Encrypted / decrypted bits per processor cycle')
print('')
generate_table(False)
