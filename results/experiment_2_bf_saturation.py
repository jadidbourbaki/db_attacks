import matplotlib.pyplot as plt
import numpy as np
import statistics

results = [
  {"bits_per_key": 0, "hashes": 1, "keys": 64, "trials": [1, 2, 3, 4, 5], "time_ms_saturation": [0.061, 0.029, 0.027, 0.027, 0.027], "minimal_set_size": [1, 1, 1, 1, 1]},
  {"bits_per_key": 1, "hashes": 1, "keys": 64, "trials": [351, 566, 890, 1257, 1527], "time_ms_saturation": [8.73, 6.047, 7.163, 6.531, 4.557], "minimal_set_size": [64, 64, 64, 64, 64]},
  {"bits_per_key": 2, "hashes": 1, "keys": 64, "trials": [1759, 1955, 2141, 2372, 2561], "time_ms_saturation": [4.436, 3.36, 3.193, 3.931, 3.106], "minimal_set_size": [128, 128, 128, 128, 128]},
  {"bits_per_key": 3, "hashes": 2, "keys": 64, "trials": [3170, 3991, 4842, 5432, 6253], "time_ms_saturation": [8.7, 10.53, 10.181, 6.778, 8.958], "minimal_set_size": [96, 96, 96, 96, 96]},
  {"bits_per_key": 4, "hashes": 2, "keys": 64, "trials": [6624, 7069, 7418, 7848, 8197], "time_ms_saturation": [4.669, 5.492, 4.478, 5.321, 4.441], "minimal_set_size": [128, 128, 128, 128, 128]},
  {"bits_per_key": 5, "hashes": 3, "keys": 64, "trials": [9148, 10163, 11017, 12007, 13057], "time_ms_saturation": [12.415, 13.272, 11.321, 12.719, 13.511], "minimal_set_size": [107, 107, 107, 107, 107]},
  {"bits_per_key": 6, "hashes": 4, "keys": 64, "trials": [17656, 21615, 26090, 29580, 34253], "time_ms_saturation": [64.232, 55.238, 62.246, 48.627, 156.815], "minimal_set_size": [96, 96, 96, 96, 96]},
  {"bits_per_key": 7, "hashes": 4, "keys": 64, "trials": [36379, 38094, 39927, 42018, 43886], "time_ms_saturation": [41.465, 27.285, 37.789, 33.943, 33.15], "minimal_set_size": [112, 112, 112, 112, 112]},
  {"bits_per_key": 8, "hashes": 5, "keys": 64, "trials": [51146, 58591, 64991, 71876, 77990], "time_ms_saturation": [125.823, 122.014, 105.091, 112.886, 101.434], "minimal_set_size": [103, 103, 103, 103, 103]},
  {"bits_per_key": 9, "hashes": 6, "keys": 64, "trials": [118062, 150281, 188281, 229722, 268361], "time_ms_saturation": [733.192, 585.977, 783.696, 749.923, 692.547], "minimal_set_size": [96, 96, 96, 96, 96]},
  {"bits_per_key": 10, "hashes": 6, "keys": 64, "trials": [279825, 298091, 312087, 325536, 343628], "time_ms_saturation": [217.544, 344.757, 264.6, 253.716, 349.093], "minimal_set_size": [107, 107, 107, 107, 107]},
]

# First plot: Total bits vs Saturation time (log scale)
plt.figure(figsize=(3.33, 2.5))  # USENIX column width

# Calculate total bits and statistics
total_bits = [r["bits_per_key"] for r in results][1:]
time_in_seconds = [[x / 1000.0 for x in r["time_ms_saturation"]] for r in results][1:]
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
plt.yticks([0.001, 0.01, 0.1, 1], ['1ms', '10ms', '0.1s', '1s'])

# Customize the plot
plt.xlabel('Bits per entry in filters')
plt.ylabel('Saturation time (s)')
plt.grid(True, linestyle='--', alpha=0.7)

# Tight layout
plt.tight_layout()

# Save first plot
plt.savefig('bloom_filter_saturation_log.pdf', dpi=300, bbox_inches='tight')

# Clear the current figure
plt.clf()

# Second plot: Total bits vs Trials with error bars
plt.figure(figsize=(3.33, 2.5))  # USENIX column width


trials_array = [r["trials"] for r in results][1:]
median_trials = [statistics.median(r) for r in trials_array]
std_trials = [statistics.stdev(r) for r in trials_array]

# Plot with error bars
plt.errorbar(total_bits, median_trials, yerr=std_trials,
            fmt='o-', 
            color='black',
            capsize=3,
            markersize=4,
            linewidth=1,
            elinewidth=1)

# Set log scale for both axes
# plt.xscale('log')
plt.yscale('log')

# Customize the plot
plt.xlabel('Bits per entry in filters')
plt.ylabel('Number of trials')
plt.grid(True, linestyle='--', alpha=0.7)

plt.yticks([1000, 10000, 100000, 1000000], ['1K', '10K', '100K', '1M'])


# Tight layout
plt.tight_layout()

# Save second plot
plt.savefig('bloom_filter_trials.pdf', dpi=300, bbox_inches='tight')