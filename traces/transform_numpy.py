#!/usr/bin/env python3

import io
from vcd.reader import TokenKind, tokenize
import math
import sys
import numpy as np
from scipy.stats import entropy

def snapshot_entropy(previous_snapshot, current_snapshot):
    global number_pages
    r = 0
    for label, previously_activated in previous_snapshot.items():
        if previously_activated != current_snapshot[label]:
            # print(label, previously_activated, snapshot[1][label])
            r += (1/number_pages) * math.log(1/number_pages)
    return r

def calculate_entropy(time_series):
    value,counts = np.unique(time_series, return_counts=True)
    return entropy(counts, base=2)

def calculate_entropy(time_series):
    value,counts = np.unique(time_series, return_counts=True)
    return entropy(counts, base=2)

def main():
    # Replace with the path to your VCD file
    vcd_file_path = "libjpeg-instrumented-relocs-bolt-page-fault-tlblur-30.vcd"
    if len(sys.argv) > 1:
        for filepath in sys.argv[1:]:
            analyze(filepath)
    else:
        analyze(vcd_file_path)

def analyze(vcd_file_path):
    signal_values = {}
    simulation_waves = []
    number_pages = 0
    result = 0
    matrix_accesses = np.array([[]])
    important_pages = set()
    n_simulation_waves = 0
    # Open and parse the VCD file
    with open(vcd_file_path, 'rb') as vcd_file:        
        # Get the existing pages
        for token in tokenize(vcd_file):
            if token.kind == TokenKind.VAR:
                signal_values[token.data.id_code] = False
            if token.kind == TokenKind.ENDDEFINITIONS:
                number_pages = len(signal_values)
            if token.kind == TokenKind.CHANGE_SCALAR and token.data.value == '1':
                important_pages.add(token.data.id_code)
            if token.kind == TokenKind.CHANGE_TIME:
                n_simulation_waves += 1

    # Don't track the pages that don't change their value
    to_del = []
    for w in signal_values:
        if not w in important_pages:
            to_del.append(w)
    for w in to_del:
        del signal_values[w]

    previous_iteration = np.full((1, len(important_pages)), False)

    # Populate the data structures and calculate entropy step by step
    with open(vcd_file_path, 'rb') as vcd_file:
        for token in tokenize(vcd_file):
            if token.kind == TokenKind.CHANGE_SCALAR and token.data.id_code in important_pages:
                signal_values[token.data.id_code] = token.data.value == '1'
            if token.kind == TokenKind.CHANGE_TIME:
                l = list(signal_values.items())
                l.sort()
                this_iteration = np.array(l)[:,1]
                array_to_add = this_iteration != previous_iteration
                ar_np = np.array(array_to_add)
                # print(f"\t[ITERATION]\n{matrix_accesses}\n{ar_np}\n")
                if matrix_accesses.size > 0:
                    matrix_accesses = np.append(matrix_accesses, [ar_np], axis = 0)
                else:
                    matrix_accesses = ar_np
                previous_iteration = this_iteration

    avg_entropy = 0
    matrix_accesses = matrix_accesses.transpose()
    with np.printoptions(threshold=np.inf):
        print(matrix_accesses[0])
    for line in matrix_accesses:
        avg_entropy += calculate_entropy(line)
    print(f"{vcd_file_path} {avg_entropy}")


if __name__ == "__main__":
    main()
