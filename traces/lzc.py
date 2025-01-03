from vcdvcd import VCDVCD
from lempel_ziv_complexity import lempel_ziv_complexity
import sys

def get_binary_string(signal_changes):
    """Convert signal changes into a binary string of 0s and 1s."""
    binary_string = ''.join([change[1] for change in signal_changes])
    return binary_string

def calculate_total_entropy(vcd_file_path):
    # Parse the VCD file
    vcd = VCDVCD(vcd_file_path)
    
    # Initialize total entropy
    total_entropy = 0
    
    # Loop through each signal
    for signal in vcd.signals:
        # Get the binary string for this signal
        signal_changes = vcd[signal].tv  # time-value pairs
        binary_string = get_binary_string(signal_changes)
        
        # Compute the Lempel-Ziv complexity for the binary string
        entropy = lempel_ziv_complexity(binary_string)
        
        # Add to total entropy
        total_entropy += entropy
    
    return total_entropy

# Example usage
vcd_file_path = sys.argv[1]
total_entropy = calculate_total_entropy(vcd_file_path)
print("Total Entropy:", total_entropy)

