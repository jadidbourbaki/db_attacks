# -- Raw Data --

fprs = [0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1]
bits_per_key = [9.58506, 6.23522, 4.79253, 3.34983, 2.50591, 1.90714, 1.4427, 1.06322, 0.742372, 0.464444, 0.219294, -0]
time_ms_existing_keys = [205.212, 213.933, 209.823, 204.97, 201.658, 202.865, 201.892, 208.379, 203.564, 206.561, 205.554, 209.714]
time_ms_existing_keys_min = [195.351, 196.56, 198.381, 194.471, 199.114, 200.033, 200, 202.327, 201.602, 202.399, 200.903, 199.423]
time_ms_existing_keys_max = [252.743, 226.077, 234.46, 414.578, 243.269, 207.498, 243.953, 270.656, 207.593, 233.791, 254.243, 220.397]
time_ms_non_existing_keys = [33.092, 44.525, 58.823, 84.339, 108.846, 137.573, 163.82, 191.962, 200.414, 287.584, 278.182, 269.44]
time_ms_non_existing_keys_min = [32.893, 44.195, 57.181, 83.79, 107.992, 135.902, 155.589, 181.906, 187.935, 264.211, 262.294, 265.263]
time_ms_non_existing_keys_max = [39.601, 50.435, 61.451, 116.707, 323.478, 257.759, 194.877, 244.703, 306.549, 301.931, 321.389, 274.918]
time_ms_existing_keys_stddev = [23.8634, 10.8804, 16.5918, 93.8656, 19.3665, 2.94152, 24.9982, 28.0515, 2.19215, 12.9052, 22.1837, 7.48845]
time_ms_non_existing_keys_stddev = [2.91883, 2.72676, 1.42786, 14.4827, 96.9187, 58.2692, 14.5019, 29.4043, 48.6895, 15.0053, 22.4656, 3.58882]

prp_time_ms_existing_keys = [217.243]
prp_time_ms_existing_keys_min = [204.636]
prp_time_ms_existing_keys_max = [232.517]
prp_time_ms_non_existing_keys = [59.705]
prp_time_ms_non_existing_keys_min = [58.34]
prp_time_ms_non_existing_keys_max = [62.069]
prp_time_ms_existing_keys_stddev = [9.39608]
prp_time_ms_non_existing_keys_stddev = [1.28264]

import matplotlib.pyplot as plt
import numpy as np

# Define data (converted to microseconds)
fprs = np.array([0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1])
total_queries = 50000  # Normalize time to µs/query

# Convert times to microseconds per query
time_existing = np.array(time_ms_existing_keys) * 1000 / total_queries
time_non_existing = np.array(time_ms_non_existing_keys) * 1000 / total_queries
time_existing_prp = np.full_like(fprs, prp_time_ms_existing_keys[0] * 1000 / total_queries)  # PRP is constant
time_non_existing_prp = np.full_like(fprs, prp_time_ms_non_existing_keys[0] * 1000 / total_queries)  # PRP is constant

# Standard deviations (converted to µs/query)
stddev_existing = np.array(time_ms_existing_keys_stddev) * 1000 / total_queries
stddev_non_existing = np.array(time_ms_non_existing_keys_stddev) * 1000 / total_queries
stddev_existing_prp = np.full_like(fprs, prp_time_ms_existing_keys_stddev[0] * 1000 / total_queries)
stddev_non_existing_prp = np.full_like(fprs, prp_time_ms_non_existing_keys_stddev[0] * 1000 / total_queries)

def plot_lookup_time(fprs, time_leveldb, stddev_leveldb, time_prp, stddev_prp, filename, ylim1 = None, ylim2 = None):
    """Generate log-log line plot for lookup times with error bars."""
    fig, ax = plt.subplots(figsize=(3.33, 2.5))  # USENIX-style wide figure

    # Plot LevelDB
    ax.errorbar(fprs, time_leveldb, yerr=stddev_leveldb, fmt='x-.', color='black', 
                capsize=3, label='RocksDB')

    # Plot LilyDB (PRP)
    ax.errorbar(fprs, time_prp, yerr=stddev_prp, fmt='.--', color='blue', 
                capsize=3, label='Secure RocksDB')

    # Labels and log scale
    # ax.set_xscale('log')
    # ax.set_yscale('log')
    ax.set_xlabel('Bloom filter false positive rate ($\epsilon$)')
    ax.set_ylabel('Lookup latency ($\mu$s)')
    ax.legend(frameon=False, loc='best')  # No box around legend

    # Grid and formatting
    ax.grid(True, which="both", linestyle="dashed", linewidth=0.5, alpha=0.6)
    # ax.spines['top'].set_visible(False)
    # ax.spines['right'].set_visible(False)

    if ylim1 is not None and ylim2 is not None:
        plt.ylim(ylim1, ylim2)

    # Save figure
    fig.tight_layout()
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close(fig)

# Generate separate plots
plot_lookup_time(fprs, time_existing, stddev_existing, time_existing_prp, stddev_existing_prp, 'rocksdb_lily_db_existing_keys.pdf', 0, 7)

plot_lookup_time(fprs, time_non_existing, stddev_non_existing, time_non_existing_prp, stddev_non_existing_prp, 'rocksdb_lily_db_non_existing_keys.pdf', 0, 7)
