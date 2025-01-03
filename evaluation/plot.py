#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
import subprocess

FULL_PLOT = False
# FULL_PLOT = True

out_name = f"plot.pdf"

scale_factor = 0.65
font_size = 14*scale_factor

x_size = 18
y_size = 3.5
opacity = 0.99

bar_width = 0.3

plt.rcParams['font.size'] = font_size


hatches = ['','','..','///','','','','','','']

experiments = [
    "micro-stressmem",
    "micro-fibonacci",
    # "micro-long-func",
    "micro-long-loop",
    "libjpeg-birds-gray",
    "yescrypt-yescrypt",
    # "yescrypt-scrypt",
    "yescrypt-pbkdf2-sha256",
    "wolfssl-pbkdf2",
    "wolfssl-aes-cbc",
    "openssl-aes-gcm",
    "openssl-rsa",
]

experiment_names = [
    "micro-stressmem",
    "micro-fibonacci",
    # "micro-long-func",
    "micro-long-loop",
    "libjpeg-birds",
    "yescrypt-yescrypt",
    # "yescrypt-scrypt",
    "yescrypt-pbkdf2",
    "wolfssl-pbkdf2",
    "wolfssl-aes-cbc",
    "openssl-aes-gcm",
    "openssl-rsa",
]

variants = [
    # "",
    "-relocs-bolt-base",
] + ([
    "-relocs-bolt",
    "-relocs-bolt-16KiB",
    # "-relocs-bolt-jmp-tgts-eflags",
    "-relocs-bolt-jmp-tgts",
] if FULL_PLOT else []) + [
    "-relocs-bolt-opt",

    "-instrumented-relocs-bolt-base",
] + ([
    "-instrumented-relocs-bolt",
    "-instrumented-relocs-bolt-16KiB",
    # "-instrumented-relocs-bolt-jmp-tgts-eflags",
    "-instrumented-relocs-bolt-jmp-tgts",
] if FULL_PLOT else []) + [
    "-instrumented-relocs-bolt-opt",
]

variant_labels = [
    "baseline",

] + ([
    "code page",
    "code page (16KiB pages)",
    # "code page (jump targets with push eflags)",
    "code page (jump targets)",
] if FULL_PLOT else []) + [
    "binary rewriting instrumentation",

    "compiler-introduced instrumentation",

] + ([
    "code and data",
    "code and data (16KiB pages)",
    # "code and data (jump targets with push eflags)",
    "code and data (jump targets)",
] if FULL_PLOT else []) + [
    "compiler-introduced and binary rewriting instrumentation",
]

# Directory containing your CSV files
csv_directory = "out"
# csv_directory = "out_bak3"

# List to store the averages for each variant
averages = []
stddevs = []

# Iterate through each experiment
for exp_name in experiments:
    exp_data_mean = []
    exp_data_stddev = []
    # Iterate through each variant
    for variant in variants:
        # Load the CSV file
        filename = os.path.join(csv_directory, f"{exp_name}{variant}.csv")
        print(filename)
        if os.path.exists(filename):
            df = pd.read_csv(filename)
            # Calculate the average and append to exp_data
            exp_data_mean.append(
                df["cycles"].mean()
            )  # Replace 'column_name' with your actual column name
            exp_data_stddev.append(np.std(df["cycles"]))
        else:
            print(f"Missing file {filename}")
            exp_data_mean.append(None)
            exp_data_stddev.append(None)
    averages.append(exp_data_mean)
    stddevs.append(exp_data_stddev)

# Plotting the bar chart
fig, ax = plt.subplots(figsize=(x_size*scale_factor, y_size*scale_factor))
ax.grid(axis='y', alpha=0.5)
index = range(1, len(experiments) + 1)

# print(averages)

# print(stddevs)

overheads = []
std_devs = []
for i in range(len(averages)):
    avg_set = averages[i]
    print(avg_set)
    base_avg = avg_set[0]
    normalized_avg_set = [avg / base_avg for avg in avg_set]
    normalized_stddev_set = [std_dev / base_avg for std_dev in stddevs[i]]
    overheads.append(normalized_avg_set)
    std_devs.append(normalized_stddev_set)

print(overheads)

for i in range(1,len(variants)):
    bars = plt.bar(
        [x * 1.4 + i * bar_width - (bar_width/2) for x in index],
        [avg[i] for avg in overheads],
        bar_width,
        yerr=[stddev[i] for stddev in std_devs],
        hatch=hatches[i],
        color=plt.cm.tab20c(i*4+1),
        error_kw=dict(lw=1, capsize=3, capthick=1, ecolor=plt.cm.tab20c(i*4), alpha=0.8),
        alpha=opacity,
        label=variant_labels[i],
    )
    ax.bar_label(bars, fmt="%.2f", fontsize=font_size*3/4)

# Calculate the geometric mean for each variant
geo_means = []
for i in range(1, len(variants)):
    variant_data = [overhead[i] for overhead in overheads]
    geo_mean = np.exp(np.mean(np.log([val for val in variant_data if val > 0])))
    geo_means.append(geo_mean)

# Add horizontal lines for geometric means
for i, geo_mean in enumerate(geo_means):
    ax.axhline(geo_mean, color=plt.cm.tab20c((i+1)*4+1), linestyle='--', linewidth=2)

ax.margins(x=0.01, y=0.05)

# plt.xlabel("Benchmark")
plt.ylabel("Slowdown factor", fontsize=font_size)
plt.ylim(bottom=1)
plt.yticks(fontsize=font_size)
plt.xticks([x * 1.4 + 1.5 * bar_width for x in index], experiment_names, fontsize=font_size)
plt.legend(fontsize=font_size, loc='best')

plt.tight_layout()
plt.savefig("margins_"+out_name, dpi=300)
plt.close()

data = {
    'Benchmark': experiment_names,
}

ps = subprocess.Popen(('/usr/bin/pdfcrop', "margins_"+out_name,
                       out_name))
ps.wait()


# Create the DataFrame
for i in range(1, len(variants)):
    data[variant_labels[i]] = [overheads[j][i] for j in range(len(overheads))]

df = pd.DataFrame(data)


# Assuming the first column is a string column
first_column = df.iloc[:, 0]  # Store the first column
numeric_df = df.iloc[:, 1:]   # Extract the numeric columns

# Calculate the geometric mean for the numeric columns
geo_mean = numeric_df.apply(lambda x: np.exp(np.mean(np.log(x))), axis=0)

# Add the geometric mean as a new row
geo_mean_row = pd.Series([None] + geo_mean.tolist(), index=df.columns)
df.loc['Geometric Mean'] = geo_mean_row

# Print the DataFrame in LaTeX format
print(df.to_latex(index=False, float_format="%.2f"))
