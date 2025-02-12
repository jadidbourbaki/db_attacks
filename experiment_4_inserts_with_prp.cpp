#include "prp_util.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/options.h>
#include <random>
#include <vector>

#define PRP

const std::string DB_PATH = "leveldb_benchmark";
const int NUM_KEYS = 50000;
const int EXPERIMENT_ITERATION_COUNT = 5;

std::string generate_key(int i) { return "key_" + std::to_string(i); }

std::string generate_value() { return "value_data"; }

double compute_std_dev(const std::vector<double> &times, double mean) {
  double sum_sq_diff = 0.0;
  for (double time : times) {
    sum_sq_diff += (time - mean) * (time - mean);
  }
  return std::sqrt(sum_sq_diff / times.size());
}

int main() {
#ifdef PRP
  std::string secret_key = "0123456789abcdef"; // 16 bytes
  StringPRP prp(secret_key);
#endif // PRP

  std::vector<double> times;

  for (int iter = 0; iter < EXPERIMENT_ITERATION_COUNT; iter++) {
    leveldb::Options options;
    options.create_if_missing = true;

    // Open database
    leveldb::DB *db;
    leveldb::Status status = leveldb::DB::Open(options, DB_PATH, &db);
    if (!status.ok()) {
      std::cerr << "Failed to open database: " << status.ToString()
                << std::endl;
      return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Insert 50K keys
    for (int i = 0; i < NUM_KEYS; i++) {
      std::string key = generate_key(i);
#ifdef PRP
      key = prp.permute(key);
#endif // PRP

      status = db->Put(leveldb::WriteOptions(), key, generate_value());
      if (!status.ok()) {
        std::cerr << "Error writing key: " << status.ToString() << std::endl;
        delete db;
        return 1;
      }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double time_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                         end_time - start_time)
                         .count() /
                     1000.0;
    times.push_back(time_ms);

    delete db;
    std::filesystem::remove_all(DB_PATH);

    std::cout << "Iteration " << iter + 1 << ": " << time_ms << " ms"
              << std::endl;
  }

  std::sort(times.begin(), times.end());

  double mean_time =
      std::accumulate(times.begin(), times.end(), 0.0) / times.size();
  double median_time = times[2];
  double std_dev = compute_std_dev(times, mean_time);

  std::cout << "Median time: " << median_time << " ms" << std::endl;
  std::cout << "Standard deviation: " << std_dev << " ms" << std::endl;

  return 0;
}
