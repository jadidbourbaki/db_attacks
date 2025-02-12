import matplotlib.pyplot as plt
import numpy as np
import statistics

results = [
  {"bits_per_key": 1, "hashes": 1, "keys": 256, "trials": [2396, 3550, 5303, 6896, 8186], "time_ms_saturation": [89.519, 29.301, 42.731, 39.584, 32.151], "minimal_set_size": [256, 256, 256, 256, 256]},
  {"bits_per_key": 1, "hashes": 1, "keys": 512, "trials": [12042, 16409, 21105, 24079, 27841], "time_ms_saturation": [174.082, 195.226, 209.093, 137.322, 170.187], "minimal_set_size": [512, 512, 512, 512, 512]},
  {"bits_per_key": 1, "hashes": 1, "keys": 1024, "trials": [36085, 47854, 53891, 61277, 66939], "time_ms_saturation": [710.301, 993.625, 535.72, 745.269, 538.499], "minimal_set_size": [1024, 1024, 1024, 1024, 1024]},
  {"bits_per_key": 1, "hashes": 1, "keys": 2048, "trials": [85463, 99006, 117249, 133894, 150240], "time_ms_saturation": [3098.47, 2318.82, 3056.56, 2855.04, 2754], "minimal_set_size": [2048, 2048, 2048, 2048, 2048]},
  {"bits_per_key": 1, "hashes": 1, "keys": 4096, "trials": [191278, 222009, 250849, 285322, 321370], "time_ms_saturation": [13426, 10345.4, 9907.38, 12295.1, 12089.9], "minimal_set_size": [4096, 4096, 4096, 4096, 4096]},
]
# First plot: Total bits vs Saturation time (log scale)
plt.figure(figsize=(3.33, 2.5))  # USENIX column width

# Calculate total bits and statistics
total_bits = [r["keys"] for r in results]
time_in_seconds = [[x / 1000.0 for x in r["time_ms_saturation"]] for r in results]
median_times = [statistics.median(r) for r in time_in_seconds]
std_times = [statistics.stdev(r) for r in time_in_seconds]

# Plot with error bars
plt.errorbar(total_bits, median_times, yerr=std_times, 
            fmt='o-',  # Line with circle markers
            color='black',  # Black for print-friendly
            capsize=3,  # Error bar cap size
            markersize=4,  # Marker size
            linewidth=1,  # Line width
            elinewidth=1)  # Error bar line width

# Set log scale for both axes
plt.yscale('log')
# plt.xscale('log')

# Set specific y-ticks for time points
# 1 ms = 1
# 1 second = 1000 ms
# 10 seconds = 10000 ms
plt.yticks([0.01, 0.1, 1, 10], ['10ms', '0.1s', '1s', '10s'])
plt.xticks([0, 1024, 2048, 4096], ['0', '1kb', '2kb', '4kb'])

# Customize the plot
plt.xlabel('Bits in Bloom Filter')
plt.ylabel('Saturation time')
plt.grid(True, linestyle='--', alpha=0.7)

# Tight layout
plt.tight_layout()

# Save first plot
plt.savefig('bloom_filter_saturation_log.pdf', dpi=300, bbox_inches='tight')