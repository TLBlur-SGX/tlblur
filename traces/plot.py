import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from vcdvcd import VCDVCD
import sys
import os

plt.rcParams.update({'font.size': 20})

def plot_vcd(vcd_file, output_file, x_min, x_max, output_dir, x_range, y_ranges, fixed_x, highlight_range=None, rectangle_bounds=None, highlight_traces=[], vlines=[], ignore=[], labels=[]):
    output_file = os.path.join(output_dir, f'{output_file}.png')
    if os.path.exists(output_file):
        print("file exists, skipping ...")
        return

    # Load the VCD file
    vcd = VCDVCD(vcd_file)

    # Create a new figure
    plt.figure(figsize=(16, len(vcd.signals) * 0.001))  # Adjust the size as needed

    # Initialize y-position
    y_position = 1
    gap_size = 2
    y_positions = []
    y_labels = []
    active_signals = []
    gap_positions = []
    y_labels_extra = []

    # Find the maximum time point in all signals
    max_time = 0
    for signal in vcd.signals:
        signal_data = vcd[signal].tv
        if signal_data:
            max_time = max(max_time, signal_data[-1][0])

    for range_idx, (start, end, label) in enumerate(y_ranges):
        first_signal_in_range = True 
        range_start_y_position = y_position 

        # Highlight the current range if it matches the highlight_range
        if highlight_range and (highlight_range[0] == start and highlight_range[1] == end):
            plt.axhspan(range_start_y_position - gap_size / 2 - 0.5, y_position + (end - start) + gap_size / 2 + 0.5,
                        color='yellow', alpha=0.1)  # Highlight with a light yellow color

        # Iterate over all signals and plot a dot for each "high" value
        for signal in vcd.signals:
            if signal == "trace.erip":
                continue

            # Check if the signal is within the specified ranges
            signal_num = int(signal[7:])
            if not (start <= signal_num <= end):
                continue

            if signal_num in ignore:
                y_position += 1
                continue

            for num, label in labels:
                if num == signal_num:
                    y_labels_extra.append((y_position, label))

            if signal_num in highlight_traces:
                plt.axhspan(y_position - 0.5, y_position + 0.5,
                            color='red', edgecolor=None, alpha=0.2)  # Highlight with a light red color

            signal_data = vcd[signal].tv  # time-value pairs for the selected signal
        
            # Extract times and values
            times = [time for time, value in signal_data]
            values = [int(value) for time, value in signal_data]

            # Skip signals that never go high
            # if not any(values):
            #     continue

            # Extend the signal to the max_time if necessary
            if times and times[-1] < max_time:
                times.append(max_time)
                values.append(values[-1])  # Repeat the last value

            # x_r = range(1, len(times))
            # if x_range is not None:
            #     x_r = crop_range_with_range(x_r, x_range)

            # Plot horizontal lines only where the signal is high
            for i in range(1, len(times)):
                if values[i-1] == 1:
                    plt.plot([times[i-1], times[i]], [y_position, y_position], color='black')
        
            # Store the active signal name for y-axis labeling
            active_signals.append(signal)

            # Add y-ticks for the first and last signals in the range
            if first_signal_in_range:
                y_positions.append(y_position)
                y_labels.append(signal[7:])
                first_signal_in_range = False

            last_signal_in_range = signal_num == end
            if last_signal_in_range:
                y_positions.append(y_position)
                y_labels.append(signal[7:])

            # Increment y_position for the next signal
            y_position += 1

        # Calculate and add a label in the middle of the current range
        # range_mid_y_position = (range_start_y_position + y_position - 1) / 2
        # plt.text(x_min - (x_max - x_min) * 0.02, range_mid_y_position, label,
        #          ha='right', va='center', fontsize=10, fontstyle='italic', color='black')

        # Add a gap between ranges, except after the last range
        if range_idx < len(y_ranges) - 1:
            gap_positions.append(y_position - 0.5)  # Store the position just before the gap
            y_position += gap_size  # Increase the gap size

    for vline in vlines:
        plt.axvline(x=vline)

    # Set labels and ticks
    plt.xlabel('Observations')
    plt.yticks(y_positions, y_labels)  # Label y-axis only with active signal names
    # plt.title(f'{vcd_file}')
    if x_range:
        plt.xlim(x_range.start, x_range.stop)
    else:
        if fixed_x:
            plt.xlim(x_min, x_max)  # Set consistent x-axis limits
        else:
            plt.xlim(left=0)
    plt.ylim(0, y_position)

    # Get current x-axis limits after plotting
    x_min, x_max = plt.gca().get_xlim()

    # Add mid-range labels
    for range_idx, (start, end, label) in enumerate(y_ranges):
        range_start_y_position = y_positions[range_idx * 2]
        range_end_y_position = y_positions[range_idx * 2 + 1]
        range_mid_y_position = (range_start_y_position + range_end_y_position) / 2
        if label == "Code":
            range_mid_y_position -= 5
        plt.text(x_max + (x_max - x_min) * 0.01, range_mid_y_position, label,
                 ha='left', va='center', fontstyle='italic', color='black', rotation=90)

    for y_pos, label in y_labels_extra:
        plt.text(x_max - (x_max - x_min) * 0.068, y_pos + 1, label,
                 ha='left', va='center', color='black', fontsize=16)

    # Add dotted lines to indicate the gaps
    for gap_pos in gap_positions:
        plt.axhline(gap_pos + 1, color='black', linestyle='dotted', linewidth=1)
    
    plt.grid(False)

    # Draw a rectangle over the plot, bounded by two x-values and extending above the top of the plot
    if rectangle_bounds:
        rect_xmin, rect_xmax = rectangle_bounds
        rect_ymin = 0.5  # Starting at the bottom of the y-axis
        rect_ymax = y_position - 0.5
        rect = Rectangle((rect_xmin, rect_ymin), rect_xmax - rect_xmin, rect_ymax - rect_ymin,
                         linewidth=2, edgecolor='black', facecolor='none')
        plt.gca().add_patch(rect)

    plt.tight_layout()

    # Save the figure to a file
    plt.savefig(output_file, dpi=300)
    # plt.show()
    plt.close()  # Close the figure to free memory

def main(vcd_files, x_range, y_ranges, fixed_x, highlight_traces=[], ignore=[], labels=[]):
    all_times = []

    # Determine the global x-axis limits by processing all VCD files
    for vcd_file, _, _, _ in vcd_files:
        vcd = VCDVCD(vcd_file)
        for signal in vcd.signals:
            signal_data = vcd[signal].tv
            times = [time for time, value in signal_data]
            all_times.extend(times)
    
    x_min, x_max = min(all_times), max(all_times)

    # Create an output directory if it doesn't exist
    output_dir = 'vcd_plots'
    os.makedirs(output_dir, exist_ok=True)

    # Plot each VCD file and save to a separate file
    for vcd_file, output_file, rectangle_bounds, vlines in vcd_files:
        if output_file is None:
            continue
        print(f'{vcd_file} -> {output_file}')
        plot_vcd(vcd_file, output_file, x_min, x_max, output_dir, x_range, y_ranges, fixed_x, y_ranges[1], rectangle_bounds, highlight_traces, vlines, ignore, labels)

if __name__ == "__main__":
    # rsa
    ranges_rsa = [
        (23, 33, "Data"),      # Global data
        (5120, 5140, "Code"),  # Code
        (5480, 5484, "Stack")  # Stack
    ]

    vcd_files_rsa_zoom = [
        ("rsa-relocs-bolt-base-page-fault-aexnotify-0.vcd", "rsa-zoom-aexnotify", (167, 228), []),
    ]

    highlight_rsa = [5134, 5136, 5138]
    labels_rsa = [(5134, "modpow"), (5136, "square"), (5138, "multiply")]

    main(vcd_files_rsa_zoom, None, ranges_rsa, True, highlight_rsa, [5481], labels_rsa)

    ranges_rsa_instrumented = [
        (23, 33, "Data"),      # Global data
        (5120, 5140, "Code"),  # Code
        (5481, 5485, "Stack")  # Stack
    ]

    vcd_files_rsa_zoom_instrumented = [
        ("rsa-relocs-bolt-base-page-fault-aexnotify-0.vcd", None, None, []),
        ("rsa-instrumented-relocs-bolt-page-fault-tlblur-1.vcd", "rsa-zoom-tlblur-1", (139, 158), []),
        ("rsa-instrumented-relocs-bolt-page-fault-tlblur-2.vcd", "rsa-zoom-tlblur-2", (139, 158), []),
        ("rsa-instrumented-relocs-bolt-page-fault-tlblur-3.vcd", "rsa-zoom-tlblur-3", (139, 155), []),
    ]

    highlight_rsa_instrumented = [5134, 5137, 5139]
    labels_rsa_instrumented = [(5134, "modpow"), (5137, "square"), (5139, "multiply")]
    
    main(vcd_files_rsa_zoom_instrumented, None, ranges_rsa_instrumented, True, highlight_rsa_instrumented, [5482], labels_rsa_instrumented)

    vcd_files_rsa = [
        ("rsa-relocs-bolt-base-single-step-base.vcd", "rsa-single-step", (1770, 2077), []),
        ("rsa-relocs-bolt-base-page-fault-aexnotify-0.vcd", "rsa-aexnotify", (167, 228), []),
    ]

    main(vcd_files_rsa, None, ranges_rsa, True, highlight_rsa, [5481], labels_rsa)
    
    # libjpeg
    ranges_libjpeg_instrumented = [
        (5223, 5243, "Code"),
        (5292, 5322, "Heap"),
        (9465, 9469, "Stack"),
    ]

    vcd_files_libjpeg_zoom_instrumented = [
        ("libjpeg-relocs-bolt-base-page-fault-aexnotify-0.vcd", None, None, []),
        ("libjpeg-instrumented-relocs-bolt-page-fault-tlblur-10.vcd", "libjpeg-tlblur-10", (11200, 11950), []),
        ("libjpeg-instrumented-relocs-bolt-page-fault-tlblur-20.vcd", "libjpeg-tlblur-20", (617, 638), []),
        ("libjpeg-instrumented-relocs-bolt-page-fault-tlblur-30.vcd", "libjpeg-tlblur-30", (580, 587), []),
    ]

    main(vcd_files_libjpeg_zoom_instrumented, None, ranges_libjpeg_instrumented, False, [], [9466])

    ranges_libjpeg = [
        (5145, 5165, "Code"),
        (5177, 5207, "Heap"),
        (9350, 9354, "Stack"),
    ]

    vcd_files_libjpeg = [
        ("libjpeg-relocs-bolt-base-single-step-base.vcd", "libjpeg-single-step", (205000, 295000), []),
        ("libjpeg-relocs-bolt-base-page-fault-aexnotify-0.vcd", "libjpeg-aexnotify", (31500, 43500), []),
    ]

    main(vcd_files_libjpeg, None, ranges_libjpeg, False, [], [9351])

    # main([("libjpeg-relocs-bolt-base-single-step-base.vcd", "libjpeg-single-step-zoom")], range(200000, 400000), ranges_libjpeg, False)
    # main([("libjpeg-relocs-bolt-base-single-step-base.vcd", "libjpeg-single-step-zoom")], range(275000, 305000), ranges_libjpeg, False)
    main([("libjpeg-relocs-bolt-base-single-step-base.vcd", "libjpeg-single-step-zoom", None, [])], range(210000, 290000), ranges_libjpeg, False, [], [9351])

    # main([("libjpeg-relocs-bolt-base-page-fault-aexnotify-0.vcd", "libjpeg-aexnotify-zoom")], range(41000, 47000), ranges_libjpeg, False)
    main([("libjpeg-relocs-bolt-base-page-fault-aexnotify-0.vcd", "libjpeg-aexnotify-zoom", None, [])], range(32000, 43000), ranges_libjpeg, False, [], [9351])

    main([("libjpeg-instrumented-relocs-bolt-page-fault-tlblur-10.vcd", "libjpeg-tlblur-10-zoom", None, [])], range(11300, 11850), ranges_libjpeg_instrumented, False, [], [9466])
    main([("libjpeg-instrumented-relocs-bolt-page-fault-tlblur-20.vcd", "libjpeg-tlblur-20-zoom", None, [])], range(620, 635), ranges_libjpeg_instrumented, False, [], [9466])
    main([("libjpeg-instrumented-relocs-bolt-page-fault-tlblur-30.vcd", "libjpeg-tlblur-30-zoom", None, [])], range(582, 585), ranges_libjpeg_instrumented, False, [], [9466])
