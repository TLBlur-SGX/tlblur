import os
import glob
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Function to calculate geometric mean
def geometric_mean(data):
    return np.exp(np.log(data).mean())

scale_factor = 0.65
font_size = 14*scale_factor

x_size = 9
y_size = 3.5

plt.rcParams['font.size'] = font_size
plt.rcParams['pdf.fonttype'] = 42
plt.rcParams['ps.fonttype'] = 42

# Directory containing CSV files
csv_directory = "./out"

# Initialize storage for results
results = []

# Process each CSV file
for file_path in glob.glob(os.path.join(csv_directory, "bench_n*_pws*_pam*.csv")):
    # Extract parameters from filename
    filename = os.path.basename(file_path)
    parts = filename.split('_')
    n = int(parts[1][1:])  # Extract 'n' value
    pws = int(parts[2][3:])  # Extract 'PWS' value
    pam = int(parts[3][3:-4])  # Extract 'PAM' value (strip .csv)
    
    # Load CSV data
    df = pd.read_csv(file_path)
    
    # Compute geometric mean for the 'total' column
    total_geom_mean = geometric_mean(df['total'])
    
    # Append result
    results.append((n, pws, pam, total_geom_mean))

# Convert results to DataFrame for easier plotting
results_df = pd.DataFrame(results, columns=["n", "PWS", "PAM", "GeometricMeanTotal"])

markers = ['o', 's', '^']

# Group by PAM to plot multiple lines
plt.figure(figsize=(x_size*scale_factor, y_size*scale_factor))
for i, (pam_value, group) in enumerate(results_df.groupby("PAM")):
    # Sort the group by PWS for proper plotting
    group = group.sort_values("PWS")
    plt.plot(group["PWS"], group["GeometricMeanTotal"], label=f"|PAM|={pam_value}", marker=markers[i])

# Plot formatting
# plt.title("Geometric Mean of Total Cycles by PWS for Different PAM Sizes")
plt.xlabel("PWS Size")
plt.ylabel("Geometric Mean of Cycles")
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.tight_layout()
plt.savefig("plot.pdf")
