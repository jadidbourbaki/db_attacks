# -- Raw Data --
fprs = [0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1]
bits_per_key = [9.58506, 6.23522, 4.79253, 3.34983, 2.50591, 1.90714, 1.4427, 1.06322, 0.742372, 0.464444, 0.219294, -0]
time_ms_existing_keys = [177.037, 167.993, 187.512, 229.504, 138.693, 146.324, 152.024, 144.917, 139.198, 144.762, 141.786, 140.024]
time_ms_existing_keys_min = [138.465, 138.382, 148.847, 138.44, 137.581, 135.58, 139.94, 138.608, 139.012, 138.233, 139.161, 138.971]
time_ms_existing_keys_max = [381.775, 399.285, 239.582, 347.193, 170.159, 162.864, 171.725, 182.396, 150.029, 152.874, 155.356, 154.387]
time_ms_non_existing_keys = [55.889, 74.67, 87.555, 112.746, 119.439, 153.48, 153.708, 150.169, 192.465, 198.524, 198.013, 206.267]
time_ms_non_existing_keys_min = [54.893, 62.603, 81.739, 88.588, 110.122, 152.047, 145.676, 143.348, 190.824, 184.705, 191.015, 181.555]
time_ms_non_existing_keys_max = [149.55, 103.095, 161.706, 134.556, 201.242, 181.928, 178.713, 203.63, 196.53, 279.13, 224.718, 228.814]
time_ms_existing_keys_stddev = [93.8081, 106.797, 34.0132, 91.5884, 14.2831, 9.75098, 10.3606, 17.4611, 6.20702, 5.11199, 6.29469, 7.69058]
time_ms_non_existing_keys_stddev = [42.2621, 18.2421, 36.39, 17.7775, 38.9472, 12.8984, 11.8314, 24.106, 2.00395, 38.259, 16.9608, 16.097]

prp_time_ms_existing_keys = [196.212]
prp_time_ms_existing_keys_min = [185.193]
prp_time_ms_existing_keys_max = [213.317]
prp_time_ms_non_existing_keys = [81.839]
prp_time_ms_non_existing_keys_min = [79.631]
prp_time_ms_non_existing_keys_max = [102.604]
prp_time_ms_existing_keys_stddev = [10.3756]
prp_time_ms_non_existing_keys_stddev = [11.7532]

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
                capsize=3, label='LevelDB')

    # Plot LilyDB (PRP)
    ax.errorbar(fprs, time_prp, yerr=stddev_prp, fmt='.--', color='blue', 
                capsize=3, label='Secure LevelDB')

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
plot_lookup_time(fprs, time_existing, stddev_existing, time_existing_prp, stddev_existing_prp, 'leveldb_lily_db_existing_keys.pdf', 0, 7)

plot_lookup_time(fprs, time_non_existing, stddev_non_existing, time_non_existing_prp, stddev_non_existing_prp, 'leveldb_lily_db_non_existing_keys.pdf', 0, 5)
