#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <rocksdb/db.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/options.h>
#include <rocksdb/table.h>
#include <rocksdb/write_batch.h>

#include "prp_util.h"
#include "util.h"

std::string DB_PATH = "experiment_1_rocks_db";

// To mitigate any impact of speculative execution on
// runtime if conditions, we set this as a preprocessor flag
// that is evaluated at compile time
// #define PRP

// Function to run RocksDB experiment with a given FPR
const result run_experiment(const double bloom_filter_fpr) {
#ifdef PRP
  std::string secret_key = "0123456789abcdef"; // 16 bytes
  StringPRP prp(secret_key);
#endif // PRP

  result r;
  r.bloom_filter_bits_per_key = calculate_bits_per_key(bloom_filter_fpr);
  r.bloom_filter_fpr = bloom_filter_fpr;

  // Configure RocksDB options
  rocksdb::Options options;
  options.create_if_missing = true;

  // Set Bloom filter
  rocksdb::BlockBasedTableOptions table_options;
  table_options.filter_policy.reset(
      rocksdb::NewBloomFilterPolicy(r.bloom_filter_bits_per_key));
  options.table_factory.reset(
      rocksdb::NewBlockBasedTableFactory(table_options));

  // Open database
  rocksdb::DB *db;
  rocksdb::Status status = rocksdb::DB::Open(options, DB_PATH, &db);
  if (!status.ok()) {
    std::cerr << "Failed to open database: " << status.ToString() << std::endl;
    return r;
  }

  // Insert keys in batches
  for (int i = 0; i < NUM_KEYS; i += BATCH_SIZE) {
    rocksdb::WriteBatch batch;
    for (int j = 0; j < BATCH_SIZE && (i + j) < NUM_KEYS; j++) {
      auto key = generate_key(i + j, false);
#ifdef PRP
      key = prp.permute(key);
#endif // PRP
      batch.Put(key, generate_value());
    }
    status = db->Write(rocksdb::WriteOptions(), &batch);
    if (!status.ok()) {
      std::cerr << "Error writing batch: " << status.ToString() << std::endl;
      delete db;
      return r;
    }
  }

  // Close and reopen database before reads
  delete db;
  status = rocksdb::DB::Open(options, DB_PATH, &db);
  if (!status.ok()) {
    std::cerr << "Failed to reopen database: " << status.ToString()
              << std::endl;
    return r;
  }

  // Measure read time for existing keys
  auto start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < QUERY_COUNT; i++) {
    std::string key = generate_key(rand() % NUM_KEYS, false);
#ifdef PRP
    key = prp.permute(key);
#endif // PRP
    std::string value;
    status = db->Get(rocksdb::ReadOptions(), key, &value);
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  r.time_ms_existing_keys =
      std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                            start_time)
          .count() /
      1000.0;

  // Measure read time for non-existent keys
  start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < QUERY_COUNT; i++) {
    std::string key = generate_key(rand() % NUM_KEYS, true);
#ifdef PRP
    key = prp.permute(key);
#endif // PRP

    std::string value;
    status = db->Get(rocksdb::ReadOptions(), key, &value);
  }

  end_time = std::chrono::high_resolution_clock::now();
  r.time_ms_non_existing_keys =
      std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                            start_time)
          .count() /
      1000.0;

  // Close database and delete files
  delete db;
  std::filesystem::remove_all(DB_PATH);

  return r;
}

int main() {
#ifdef PRP
  std::cout << "PRP Enabled" << std::endl;
#else
  std::cout << "PRP Disabled" << std::endl;
#endif // PRP
  // set the seed for reproducibility. this can be changed
  srand(42);

#ifdef PRP
  // We only really want to know the PRP's performance at a low FPR as it isn't
  // affected by adversarial input as per Moni Naor, Yogev Eylon et al. ("Bloom
  // Filters in Adversarial Environments").
  auto fprs = {0.01};
#else
  auto fprs = {0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
#endif // PRP

  std::vector<result> results;

  for (auto fpr : fprs) {
    std::vector<result> temp_results;

    for (int i = 0; i < EXPERIMENT_ITERATION_COUNT; i++) {
      temp_results.push_back(run_experiment(fpr));
      std::cout << " . " << std::flush;
    }

    std::cout << " * " << std::flush;

    results.push_back(median_results(temp_results));
  }

  std::cout << std::endl;

  summarize(results);

  return 0;
}
