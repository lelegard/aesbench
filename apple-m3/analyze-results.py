#!/usr/bin/env python
#----------------------------------------------------------------------------
# aesbench - Copyright (c) 2024, Thierry Lelegard
# BSD 2-Clause License, see LICENSE file.
# Analyze results files and produce an analysis in RESULTS.txt.
#----------------------------------------------------------------------------

import os, sys
dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, os.path.dirname(dir))
import analyze

# List of CPU cores and corresponding result files.
results = [
    {'os': 'macOS', 'cpu': 'Apple M3', 'frequency': 4.00, 'file': 'result-macos.txt'},
    {'os': 'Linux', 'cpu': 'Apple M3', 'frequency': 4.00, 'file': 'result-linux-1.txt'},
    {'os': 'Linux', 'cpu': 'Apple M3', 'frequency': 4.00, 'file': 'result-linux-2.txt'}
]

# Column headers.
headers = {'os': 'System', 'openssl': 'OpenSSL'}

# Main code.
if __name__ == '__main__':
    algos = analyze.load_results(results, dir)
    with open(dir + '/RESULTS.txt', 'w') as output:
        analyze.display_tables(results, algos, headers, output)
