import matplotlib.pyplot as plt
import numpy as np

# LevelDB Data
fprs = np.array([0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1])
time_ms_existing_keys = np.array([150.013, 158.871, 140.878, 152.119, 155.298, 149.068, 151.968, 147.871, 156.027, 154.08, 140.513, 143.226])
time_ms_existing_keys_stddev = np.array([18.3846, 39.1511, 8.96284, 8.13547, 14.596, 18.4995, 32.9437, 30.8038, 32.1945, 38.602, 3.35973, 13.7466])
time_ms_non_existing_keys = np.array([86.752, 64.148, 103.33, 102.847, 112.612, 161.134, 150.745, 153.699, 203.317, 204.423, 200.424, 193.34])
time_ms_non_existing_keys_stddev = np.array([33.8351, 12.5663, 28.2212, 19.2139, 20.8368, 22.0008, 5.06553, 6.98676, 92.2371, 15.9508, 31.47, 24.198])

# Normalize time to µs/query
total_queries = 50000
time_ms_existing_keys /= total_queries / 1000
time_ms_existing_keys_stddev /= total_queries / 1000
time_ms_non_existing_keys /= total_queries / 1000
time_ms_non_existing_keys_stddev /= total_queries / 1000

# RocksDB Data
time_ms_existing_key_rocksdb = np.array([197.309, 195.509, 192.926, 192.152, 202.06, 204.879, 213.66, 205.62, 208.755, 202.807, 202.687, 203.835])
time_ms_non_existing_keys_rocksdb = np.array([33.854, 45.021, 57.564, 83.55, 113.495, 133.687, 159.903, 185.715, 188.252, 264.319, 261.604, 267.759])
time_ms_existing_keys_stddev_rocksdb = np.array([7.12205, 48.472, 2.28558, 9.25831, 19.7968, 11.0496, 17.2566, 38.0366, 22.9935, 7.65524, 14.2674, 30.4808])
time_ms_non_existing_keys_stddev_rocksdb = np.array([9.09346, 2.95962, 3.62185, 31.5414, 4.63837, 4.2953, 18.9328, 59.6445, 49.4159, 13.7329, 62.4515, 29.5029])

# Normalize time to µs/query
time_ms_existing_key_rocksdb /= total_queries / 1000
time_ms_existing_keys_stddev_rocksdb /= total_queries / 1000
time_ms_non_existing_keys_rocksdb /= total_queries / 1000
time_ms_non_existing_keys_stddev_rocksdb /= total_queries / 1000

# Set figure size for PVLDB format
plt.figure(figsize=(3.33, 2.5))

# Plot RocksDB data
plt.errorbar(fprs, time_ms_non_existing_keys_rocksdb, yerr=time_ms_non_existing_keys_stddev_rocksdb, 
             label="RocksDB", 
            fmt='s--',  # Line with circle markers
            color='blue',  # Black for print-friendly
            capsize=3,  # Error bar cap size
            markersize=3,  # Marker size
            linewidth=1,  # Line width
            elinewidth=1)  # Error bar line width

# Plot LevelDB data
plt.errorbar(fprs, time_ms_non_existing_keys, yerr=time_ms_non_existing_keys_stddev, 
            label="LevelDB", 
            fmt='o-',  # Line with circle markers
            color='black',  # Black for print-friendly
            capsize=1.5,  # Error bar cap size
            markersize=3,  # Marker size
            linewidth=1,  # Line width
            elinewidth=1)  # Error bar line width

# Axis labels with LaTeX formatting
plt.xlabel("Bloom filter false positive rate ($\epsilon$)")
plt.ylabel("Lookup latency ($\mu$s)")

# Improve tick formatting
plt.xticks([0.0, *fprs[2:]], rotation=45)
plt.ylim(0, max(time_ms_non_existing_keys_rocksdb + time_ms_non_existing_keys_stddev_rocksdb) * 1.1)

# Grid and legend
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend()

# Save for PVLDB submission
plt.savefig("experiment_1_zero_result_lookup.pdf", dpi=300, bbox_inches="tight")
