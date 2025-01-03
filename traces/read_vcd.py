#!/usr/bin/env python3

import io
from vcd.reader import TokenKind, tokenize
import math
import sys

# Dictionary to store signal values
signal_values = {}

simulation_waves = []

number_pages = 0

result = 0

important_pages = set()

def snapshot_entropy(previous_snapshot, current_snapshot):
    global number_pages
    r = 0
    for label, previously_activated in previous_snapshot.items():
        if previously_activated != current_snapshot[label]:
            # print(label, previously_activated, snapshot[1][label])
            r += (1/number_pages) * math.log(1/number_pages)
    return r



def main():
    global number_pages
    global signal_values
    global simulation_waves
    global result
    global important_pages
    n_simulation_waves = 0
    # Replace with the path to your VCD file
    vcd_file_path = "libjpeg-instrumented-relocs-bolt-page-fault-tlblur-30.vcd"
    if len(sys.argv) > 1:
        vcd_file_path = sys.argv[1]

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

    number_pages = len(important_pages)
    previous_iteration = {}

    # Populate the data structures and calculate entropy step by step
    with open(vcd_file_path, 'rb') as vcd_file:
        for token in tokenize(vcd_file):
            if token.kind == TokenKind.CHANGE_SCALAR and token.data.id_code in important_pages:
                signal_values[token.data.id_code] = token.data.value == '1'
            if token.kind == TokenKind.CHANGE_TIME:
                # simulation_waves.append(signal_values.copy())
                r = snapshot_entropy(previous_iteration, signal_values)
                result += r/n_simulation_waves
                previous_iteration = signal_values.copy()
            # print(token)

    # for w in zip(simulation_waves[1:], simulation_waves):
    #     r = snapshot_entropy(w)
    #     result += r/n_simulation_waves
    print(f"[RESULT] {result}")


if __name__ == "__main__":
    main()
