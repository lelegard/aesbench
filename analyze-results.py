#!/usr/bin/env python
#----------------------------------------------------------------------------
# aesbench - Copyright (c) 2024, Thierry Lelegard
# BSD 2-Clause License, see LICENSE file.
# Analyze results files and produce an analysis in markdown format.
# With option --pprint, print the data structure.
#----------------------------------------------------------------------------

import os, sys, pprint
GIGA = 1000000000.0

# List of CPU cores and corresponding result files.
results = [
    {'cpu': 'i7-8565U',    'frequency': 4.20, 'file': 'intel-i7-8565U-linux-vm.txt'},
    {'cpu': 'i7-13700H',   'frequency': 5.00, 'file': 'intel-i7-13700H-linux-vm.txt'},
    {'cpu': 'Cortex-A53',  'frequency': 1.20, 'file': 'arm-rpi3-cortex-a53-linux.txt'},
    {'cpu': 'Cortex-A72',  'frequency': 1.80, 'file': 'arm-rpi4-cortex-a72-linux.txt'},
    {'cpu': 'Neoverse-N1', 'frequency': 3.00, 'file': 'arm-ampere-neoverse-n1-linux.txt'},
    {'cpu': 'Neoverse-V1', 'frequency': 2.60, 'file': 'arm-graviton3-neoverse-v1-linux-vm.txt'},
    {'cpu': 'Neoverse-V2', 'frequency': 3.30, 'file': 'arm-grace-neoverse-v2-linux.txt'},
    {'cpu': 'Apple-M1',    'frequency': 3.20, 'file': 'arm-apple-m1-macos.txt'},
    {'cpu': 'Apple-M3',    'frequency': 4.00, 'file': 'arm-apple-m3-macos.txt'}
]

# Main code: load all result files.
rootdir = os.path.dirname(os.path.abspath(sys.argv[0]))
algos = []
for res in results:
    res['header'] = '%s<br/>%.2f GHz' % (res['cpu'], res['frequency'])
    res['width'] = len(res['header'])
    res['data'] = {}
    with open(rootdir + '/results/' + res['file'], 'r') as input:
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
                    res['data'][algo] = {
                        'encrypt': {'bitrate': {'value': 0, 'rank': 0}, 'bitcycle': {'value': 0.0, 'rank': 0}},
                        'decrypt': {'bitrate': {'value': 0, 'rank': 0}, 'bitcycle': {'value': 0.0, 'rank': 0}}
                    }
                elif (line[0] == 'encrypt-bitrate' or line[0] == 'decrypt-bitrate') and algo is not None:
                    op = line[0].split('-')[0]
                    res['data'][algo][op]['bitrate']['value'] = int(line[1])
                    res['data'][algo][op]['bitcycle']['value'] = float(line[1]) / (res['frequency'] * GIGA)

# Build CPU rankings for each operation.
for algo in algos:
    for op in ['encrypt', 'decrypt']:
        for value in ['bitrate', 'bitcycle']:
            dlist = [(res['cpu'], res['data'][algo][op][value]['value']) for res in results]
            dlist.sort(key=lambda x: x[1], reverse=True)
            for rank in range(len(dlist)):
                res = next(r for r in results if r['cpu'] == dlist[rank][0])
                res['data'][algo][op][value]['rank'] = rank + 1

# Generate a markdown table.
# Results are either bits/second or bits/cycle.
def generate_table(value_name):
    # Compute width of first column.
    header_0 = 'CPU core<br/>Frequency'
    width_0 = len(header_0)
    for name in algos:
        width_0 = max(width_0, len(name + ' encrypt'))
    # Output headers lines.
    print('| %*s |' % (-width_0, header_0), end='')
    for res in results:
        print(' %s |' % (res['header']), end='')
    print('')
    print('| %s |' % (width_0 * '-'), end='')
    for res in results:
        print(' :%s: |' % ((res['width'] - 2) * '-'), end='')
    print('')
    # Output one line per AES operation.
    for algo in algos:
        for op in ['encrypt', 'decrypt']:
            print('| %*s |' % (-width_0, algo + ' ' + op), end='')
            for res in results:
                if algo in res['data'] and op in res['data'][algo]:
                    value = res['data'][algo][op][value_name]
                else:
                    value = {'value': 0, 'rank': 0}
                if value['value'] <= 0:
                    s = ''
                elif type(value['value']) is int:
                    s = '%.3f (%d)' % (float(value['value']) / GIGA, value['rank'])
                elif value['value'] >= 10.0:
                    s = '%.1f (%d)' % (value['value'], value['rank'])
                elif value['value'] >= 1.0:
                    s = '%.2f (%d)' % (value['value'], value['rank'])
                else:
                    s = '%.3f (%d)' % (value['value'], value['rank'])
                print(' %*s |' % (res['width'], s), end='')
            print('')

# Generate the final markdown file.
if len(sys.argv) > 1 and sys.argv[1] == "--pprint":
    pprint.pprint(results, width=132)
else:
    print('# Results comparison')
    print('')
    print('## Encryption / decryption bitrate')
    print('The encryption and decryption bitrates are in gigabits per second.')
    print('The number between parentheses is the rank of the CPU in the line.')
    print('')
    generate_table('bitrate')
    print('')
    print('## Encrypted / decrypted bits per processor cycle')
    print('The values are the numbers of processed bits per cycle.')
    print('These values give a relative value of each CPU core, independently of the frequency.')
    print('The number between parentheses is the rank of the CPU in the line.')
    print('')
    generate_table('bitcycle')
