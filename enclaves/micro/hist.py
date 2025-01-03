#!/bin/env python3

import matplotlib.pyplot as plt

def plot_histogram_combined(file_paths):
    plt.figure(figsize=(10, 6))
    for file_path in file_paths:
        with open(file_path, 'r') as file:
            numbers = [int(line.strip()) for line in file.readlines()]
            plt.hist(numbers, bins=100, alpha=0.5, label=file_path)

    plt.xlabel('Cycles')
    plt.ylabel('Frequency')
    plt.ylim(0, 15000)
    plt.xlim(0, 170000)
    plt.legend()
    plt.grid(True)
    plt.savefig("hist_combined.png", dpi=300)
    plt.close()

def plot_histogram(file_path):
    plt.figure(figsize=(10, 6))
    with open(file_path, 'r') as file:
        numbers = [int(line.strip()) for line in file.readlines()]
        plt.hist(numbers, bins=100, alpha=0.5, label=file_path)

    plt.xlabel('Cycles')
    plt.ylabel('Frequency')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"hist_{file_path}.png", dpi=300)
    plt.close()

plot_histogram_combined([
    'micro_stressmem_baseline.txt', 
    'micro_stressmem_inline.txt', 
    'micro_stressmem_outline.txt', 
    'micro_stressmem_outline_nobranch.txt',

    'micro_stressmem_outline_empty.txt',
    'micro_stressmem_outline_pushpop.txt',
    'micro_stressmem_outline_pushpopflags.txt',
    'micro_stressmem_outline_nocounter.txt',
    'micro_stressmem_outline_novtlbstore.txt',
])
plot_histogram('micro_stressmem_baseline.txt')
plot_histogram('micro_stressmem_inline.txt')
plot_histogram('micro_stressmem_outline.txt')
plot_histogram('micro_stressmem_outline_nobranch.txt')

plot_histogram('micro_stressmem_outline_empty.txt')
plot_histogram('micro_stressmem_outline_pushpop.txt')
plot_histogram('micro_stressmem_outline_pushpopflags.txt')
plot_histogram('micro_stressmem_outline_nocounter.txt')
plot_histogram('micro_stressmem_outline_novtlbstore.txt')
