#!/usr/bin/env python
#----------------------------------------------------------------------------
# aesbench - Copyright (c) 2024, Thierry Lelegard
# BSD 2-Clause License, see LICENSE file.
# Analyze results files and produce an analysis in markdown format.
# With option --text, generate raw text format.
# With option --pprint, print the data structure.
#----------------------------------------------------------------------------

import os, sys, pprint
GIGA = 1000000000.0

# List of CPU cores and corresponding result files.
results = [
    {'cpu': 'i7-8565U',    'frequency': 4.20, 'file': 'intel-i7-8565U-linux-vm.txt'},
    {'cpu': 'i7-13700H',   'frequency': 5.00, 'file': 'intel-i7-13700H-linux-vm.txt'},
    {'cpu': 'Xeon G6348',  'frequency': 2.60, 'file': 'intel-xeon-gold-6348-linux.txt'},
    {'cpu': 'Cortex A53',  'frequency': 1.20, 'file': 'arm-rpi3-cortex-a53-linux.txt'},
    {'cpu': 'Cortex A72',  'frequency': 1.80, 'file': 'arm-rpi4-cortex-a72-linux.txt'},
    {'cpu': 'Neoverse N1', 'frequency': 3.00, 'file': 'arm-ampere-neoverse-n1-linux.txt'},
    {'cpu': 'Neoverse V1', 'frequency': 2.60, 'file': 'arm-graviton3-neoverse-v1-linux-vm.txt'},
    {'cpu': 'Neoverse V2', 'frequency': 3.30, 'file': 'arm-grace-neoverse-v2-linux.txt'},
    {'cpu': 'Apple M1',    'frequency': 3.20, 'file': 'arm-apple-m1-macos.txt'},
    {'cpu': 'Apple M3',    'frequency': 4.00, 'file': 'arm-apple-m3-macos.txt'}
]

# Command line option
opt_pprint = len(sys.argv) > 1 and sys.argv[1] == "--pprint"
opt_text = len(sys.argv) > 1 and sys.argv[1] == "--text"

# Main code: load all result files.
rootdir = os.path.dirname(os.path.abspath(sys.argv[0]))
algos = []
for res in results:
    res['fr_header'] = '%.2f GHz' % (res['frequency'])
    res['md_header'] = '%s<br/>%s' % (res['cpu'], res['fr_header'])
    res['md_width'] = len(res['md_header'])
    res['txt_width'] = max(len(res['cpu']), len(res['fr_header']))
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
                        'encrypt': {'bitrate':  {'value': 0,   'string': '', 'rank': 0},
                                    'bitcycle': {'value': 0.0, 'string': '', 'rank': 0}},
                        'decrypt': {'bitrate':  {'value': 0,   'string': '', 'rank': 0},
                                    'bitcycle': {'value': 0.0, 'string': '', 'rank': 0}}
                    }
                elif (line[0] == 'encrypt-bitrate' or line[0] == 'decrypt-bitrate') and algo is not None:
                    op = line[0].split('-')[0]
                    bc = float(line[1]) / (res['frequency'] * GIGA)
                    res['data'][algo][op]['bitrate']['value'] = int(line[1])
                    res['data'][algo][op]['bitrate']['string'] = '%.3f' % (float(line[1]) / GIGA) 
                    res['data'][algo][op]['bitcycle']['value'] = bc
                    if bc >= 10.0:
                        res['data'][algo][op]['bitcycle']['string'] = '%.1f' % bc
                    elif bc >= 1.0:
                        res['data'][algo][op]['bitcycle']['string'] = '%.2f' % bc
                    else:
                        res['data'][algo][op]['bitcycle']['string'] = '%.3f' % bc

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
                res['data'][algo][op][value]['rank'] = rank + 1
                res['data'][algo][op][value]['string'] += ' (%d)' % res['data'][algo][op][value]['rank']
                l = len(res['data'][algo][op][value]['string'])
                res['md_width'] = max(res['md_width'], l)
                res['txt_width'] = max(res['txt_width'], l)

# Generate a markdown table.
def markdown_table(value_name):
    # Compute width of first column.
    header_0 = 'CPU core<br/>Frequency'
    w0 = max(width_0, len(header_0))
    # Output headers lines.
    print('| %*s |' % (-w0, header_0), end='')
    for res in results:
        print(' %s |' % (res['md_header']), end='')
    print('')
    print('| %s |' % (w0 * '-'), end='')
    for res in results:
        print(' :%s: |' % ((res['md_width'] - 2) * '-'), end='')
    print('')
    # Output one line per AES operation.
    for algo in algos:
        for op in ['encrypt', 'decrypt']:
            print('| %*s |' % (-w0, algo + ' ' + op), end='')
            for res in results:
                if algo in res['data'] and op in res['data'][algo]:
                    print(' %*s |' % (res['md_width'], res['data'][algo][op][value_name]['string']), end='')
                else:
                    print(' %*s |' % (res['md_width'], ''), end='')
            print('')

# Generate a text table.
def text_table(value_name):
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
        l1 += SEP + res['cpu'].rjust(res['txt_width'])
        l2 += SEP + res['fr_header'].rjust(res['txt_width'])
        l3 += SEP + (res['txt_width'] * '-')
    print(l1.rstrip())
    print(l2.rstrip())
    print(l3.rstrip())
    # Output one line per AES operation.
    for algo in algos:
        for op in ['encrypt', 'decrypt']:
            l = (algo + ' ' + op).ljust(w0)
            for res in results:
                if algo in res['data'] and op in res['data'][algo]:
                    l += SEP + res['data'][algo][op][value_name]['string'].rjust(res['txt_width'])
                else:
                    l += SEP + res['txt_width'] * ' '
            print(l.rstrip())

# Generate the final markdown or text file.
if opt_pprint:
    pprint.pprint(results, width=132)
elif opt_text:
    print('ENCRYPTION / DECRYPTION BITRATE (Gb/s)')
    print('')
    text_table('bitrate')
    print('')
    print('ENCRYPTED / DECRYPTED BITS PER PROCESSOR CYCLE')
    print('')
    text_table('bitcycle')
else:
    print('# Results comparison')
    print('')
    print('## Encryption / decryption bitrate')
    print('The encryption and decryption bitrates are in gigabits per second.')
    print('The number between parentheses is the rank of the CPU in the line.')
    print('')
    markdown_table('bitrate')
    print('')
    print('## Encrypted / decrypted bits per processor cycle')
    print('The values are the numbers of processed bits per cycle.')
    print('These values give a relative value of each CPU core, independently of the frequency.')
    print('The number between parentheses is the rank of the CPU in the line.')
    print('')
    markdown_table('bitcycle')
