#!/usr/bin/env python
#----------------------------------------------------------------------------
# aesbench - Copyright (c) 2024, Thierry Lelegard
# BSD 2-Clause License, see LICENSE file.
# Analyze results files and produce an analysis in RESULTS.txt.
# With option --pprint, print the data structure instead of creating the file.
#----------------------------------------------------------------------------

import os, sys, pprint
GIGA = 1000000000.0

# List of CPU cores and corresponding result files.
results = [
    {'cpu': 'i7-8565U',    'frequency': 4.20, 'file': 'intel-i7-8565U-linux-vm.txt'},
    {'cpu': 'i7-13700H',   'frequency': 5.00, 'file': 'intel-i7-13700H-linux-vm.txt'},
    {'cpu': 'Xeon G6348',  'frequency': 2.60, 'file': 'intel-xeon-gold-6348-linux.txt'},
    {'cpu': 'Xeon M9460',  'frequency': 3.50, 'file': 'intel-xeon-max-9460-linux.txt'},
    {'cpu': 'Cortex A53',  'frequency': 1.20, 'file': 'arm-rpi3-cortex-a53-linux.txt'},
    {'cpu': 'Cortex A72',  'frequency': 1.80, 'file': 'arm-rpi4-cortex-a72-linux.txt'},
    {'cpu': 'Neoverse N1', 'frequency': 3.00, 'file': 'arm-ampere-neoverse-n1-linux.txt'},
    {'cpu': 'Neoverse V1', 'frequency': 2.60, 'file': 'arm-graviton3-neoverse-v1-linux-vm.txt'},
    {'cpu': 'Neoverse V2', 'frequency': 3.30, 'file': 'arm-grace-neoverse-v2-linux.txt'},
    {'cpu': 'Apple M1',    'frequency': 3.20, 'file': 'arm-apple-m1-macos.txt'},
    {'cpu': 'Apple M3',    'frequency': 4.00, 'file': 'arm-apple-m3-macos.txt'}
]

# Main code: load all result files.
rootdir = os.path.dirname(os.path.abspath(sys.argv[0]))
algos = []
for res in results:
    res['freq_header'] = '%.2f GHz' % (res['frequency'])
    res['width'] = max(len(res['cpu']), len(res['freq_header']))
    res['data'] = {}
    with open(rootdir + '/results/' + res['file'], 'r') as input:
        algo = None
        for line in input:
            line = [field.strip() for field in line.split(':')]
            if len(line) == 2:
                if line[0] == 'algo':
                    algo = line[1]
                    if algo.startswith('id-aes'):
                        algo = 'AES-' + algo[6:]
                    if algo not in algos:
                        algos += [algo]
                    res['data'][algo] = {
                        'encrypt': {'bitrate':  {'value': 0,   'string': '', 'rank': 0},
                                    'bitcycle': {'value': 0.0, 'string': '', 'rank': 0}},
                        'decrypt': {'bitrate':  {'value': 0,   'string': '', 'rank': 0},
                                    'bitcycle': {'value': 0.0, 'string': '', 'rank': 0}}
                    }
                elif (line[0] == 'encrypt-bitrate' or line[0] == 'decrypt-bitrate') and algo is not None:
                    data = res['data'][algo][line[0].split('-')[0]]
                    bitcycle = float(line[1]) / (res['frequency'] * GIGA)
                    data['bitrate']['value'] = int(line[1])
                    data['bitrate']['string'] = '%.3f' % (float(line[1]) / GIGA) 
                    data['bitcycle']['value'] = bitcycle
                    if bitcycle >= 10.0:
                        data['bitcycle']['string'] = '%.1f' % bitcycle
                    elif bitcycle >= 1.0:
                        data['bitcycle']['string'] = '%.2f' % bitcycle
                    else:
                        data['bitcycle']['string'] = '%.3f' % bitcycle

# Width of algorithm column.
width_0 = max([len(a) for a in algos]) + len(' encrypt')

# Build CPU rankings for each operation.
for algo in algos:
    for op in ['encrypt', 'decrypt']:
        for value in ['bitrate', 'bitcycle']:
            dlist = [(res['cpu'], res['data'][algo][op][value]['value']) for res in results]
            dlist.sort(key=lambda x: x[1], reverse=True)
            for rank in range(len(dlist)):
                res = next(r for r in results if r['cpu'] == dlist[rank][0])
                data = res['data'][algo][op][value]
                data['rank'] = rank + 1
                data['string'] += ' (%d)' % data['rank']
                res['width'] = max(res['width'], len(data['string']))

# Generate a text table.
def text_table(value_name, file):
    # Separator between columns.
    SEP = '   '
    # Output headers lines.
    l1 = 'CPU core'
    l2 = 'Frequency'
    w0 = max(width_0, len(l1), len(l2))
    l1 = l1.ljust(w0)
    l2 = l2.ljust(w0)
    l3 = w0 * '-'
    for res in results:
        l1 += SEP + res['cpu'].rjust(res['width'])
        l2 += SEP + res['freq_header'].rjust(res['width'])
        l3 += SEP + (res['width'] * '-')
    print(l1.rstrip(), file=output)
    print(l2.rstrip(), file=output)
    print(l3.rstrip(), file=output)
    # Output one line per AES operation.
    for algo in algos:
        for op in ['encrypt', 'decrypt']:
            l = (algo + ' ' + op).ljust(w0)
            for res in results:
                if algo in res['data'] and op in res['data'][algo]:
                    l += SEP + res['data'][algo][op][value_name]['string'].rjust(res['width'])
                else:
                    l += SEP + res['width'] * ' '
            print(l.rstrip(), file=output)

# Generate the final markdown or text file.
if len(sys.argv) > 1 and sys.argv[1] == "--pprint":
    pprint.pprint(results, width=132)
else:
    with open(rootdir + '/RESULTS.txt', 'w') as output:
        print('ENCRYPTION / DECRYPTION BITRATE (Gb/s)', file=output)
        print('', file=output)
        text_table('bitrate', file=output)
        print('', file=output)
        print('ENCRYPTED / DECRYPTED BITS PER PROCESSOR CYCLE', file=output)
        print('', file=output)
        text_table('bitcycle', file=output)
