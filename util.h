#ifndef UTIL_H
#define UTIL_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#define NUM_KEYS 10'000'000 // More keys → More Bloom filter saturation
#define QUERY_COUNT 50'000  // Queries to measure false positive rate
#define BATCH_SIZE 10'000   // Batch inserts to avoid large transactions
#define EXPERIMENT_ITERATION_COUNT 5

struct result {
  double bloom_filter_fpr;
  double bloom_filter_bits_per_key;
  double time_ms_existing_keys;
  double time_ms_non_existing_keys;
  double time_ms_existing_keys_max;
  double time_ms_existing_keys_min;
  double time_ms_non_existing_keys_max;
  double time_ms_non_existing_keys_min;
  double time_ms_existing_keys_stddev;
  double time_ms_non_existing_keys_stddev;
};

struct insertion_result_temp {
  int n_hashes;
  int n_keys;
  int bits;
  int trials;
  double time_ms_saturation;
  size_t minimal_set_size;
};

struct insertion_result {
  int n_hashes;
  int n_keys;
  int bits;
  std::vector<int> trials;
  std::vector<double> time_ms_saturation;
  std::vector<size_t> minimal_set_size;
};

struct insertion_result
insertion_result_from_temp(const std::vector<insertion_result_temp> &tmp) {
  assert(tmp.size() != 0);
  insertion_result result;
  result.bits = tmp[0].bits;
  result.n_hashes = tmp[0].n_hashes;
  result.n_keys = tmp[0].n_keys;

  for (auto r : tmp) {
    result.minimal_set_size.push_back(r.minimal_set_size);
    result.time_ms_saturation.push_back(r.time_ms_saturation);
    result.trials.push_back(r.trials);
  }

  return result;
}

// Function to calculate bits per key from desired false positive rate (FPR) for
// a Bloom Filter.
inline double calculate_bits_per_key(double fpr) {
  return -std::log(fpr) / (std::log(2) * std::log(2));
}

// Function to generate random keys
std::string generate_key(int i, bool fake) {
  std::string key = "key_" + std::to_string(i);
  if (fake) {
    key.append("_fake");
  }

  return key;
}

inline std::string generate_value() {
  std::string value = "value";
  return value;
}

template <typename T> const T find_median(std::vector<T> &nums) {
  int n = nums.size();
  sort(nums.begin(), nums.end());

  if (n % 2 == 1) {
    return nums[n / 2];
  } else {
    return (double)(nums[n / 2 - 1] + nums[n / 2]) / 2.0;
  }
}

double calculate_stddev(const std::vector<double> &values, double mean) {
  double sum = 0.0;
  for (double val : values) {
    sum += (val - mean) * (val - mean);
  }
  return std::sqrt(sum / values.size());
}

const result median_results(const std::vector<result> &results) {
  assert(results.size() != 0);

  result r;
  r.bloom_filter_fpr = results[0].bloom_filter_fpr;
  r.bloom_filter_bits_per_key = results[0].bloom_filter_bits_per_key;

  std::vector<double> time_ms_existing_keys;
  std::vector<double> time_ms_non_existing_keys;

  for (auto result : results) {
    time_ms_existing_keys.push_back(result.time_ms_existing_keys);
    time_ms_non_existing_keys.push_back(result.time_ms_non_existing_keys);
  }

  r.time_ms_existing_keys = find_median(time_ms_existing_keys);
  r.time_ms_existing_keys_max = *std::max_element(time_ms_existing_keys.begin(),
                                                  time_ms_existing_keys.end());
  r.time_ms_existing_keys_min = *std::min_element(time_ms_existing_keys.begin(),
                                                  time_ms_existing_keys.end());

  r.time_ms_non_existing_keys = find_median(time_ms_non_existing_keys);
  r.time_ms_non_existing_keys_max = *std::max_element(
      time_ms_non_existing_keys.begin(), time_ms_non_existing_keys.end());
  r.time_ms_non_existing_keys_min = *std::min_element(
      time_ms_non_existing_keys.begin(), time_ms_non_existing_keys.end());

  r.time_ms_existing_keys_stddev =
      calculate_stddev(time_ms_existing_keys, r.time_ms_existing_keys);
  r.time_ms_non_existing_keys_stddev =
      calculate_stddev(time_ms_non_existing_keys, r.time_ms_non_existing_keys);

  return r;
}

template <typename T>
void print_vector_as_python_list(const std::string &name,
                                 const std::vector<T> &vec) {
  std::cout << name << " = ";
  std::cout << "[";
  for (size_t i = 0; i < vec.size(); i++) {
    std::cout << vec[i];
    if (i < vec.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
}

void summarize(std::vector<result> results) {
  std::vector<double> fprs;
  std::vector<double> bits_per_key;
  std::vector<double> time_ms_existing_keys;
  std::vector<double> time_ms_non_existing_keys;
  std::vector<double> time_ms_existing_keys_min;
  std::vector<double> time_ms_existing_keys_max;
  std::vector<double> time_ms_non_existing_keys_min;
  std::vector<double> time_ms_non_existing_keys_max;
  std::vector<double> time_ms_existing_keys_stddev;
  std::vector<double> time_ms_non_existing_keys_stddev;

  for (auto result : results) {
    fprs.push_back(result.bloom_filter_fpr);
    bits_per_key.push_back(result.bloom_filter_bits_per_key);
    time_ms_existing_keys.push_back(result.time_ms_existing_keys);
    time_ms_non_existing_keys.push_back(result.time_ms_non_existing_keys);
    time_ms_existing_keys_min.push_back(result.time_ms_existing_keys_min);
    time_ms_existing_keys_max.push_back(result.time_ms_existing_keys_max);
    time_ms_non_existing_keys_min.push_back(
        result.time_ms_non_existing_keys_min);
    time_ms_non_existing_keys_max.push_back(
        result.time_ms_non_existing_keys_max);

    time_ms_existing_keys_stddev.push_back(result.time_ms_existing_keys_stddev);
    time_ms_non_existing_keys_stddev.push_back(
        result.time_ms_non_existing_keys_stddev);
  }

  std::cout << "summary" << std::endl;
  print_vector_as_python_list("fprs", fprs);
  print_vector_as_python_list("bits_per_key", bits_per_key);
  print_vector_as_python_list("time_ms_existing_keys", time_ms_existing_keys);
  print_vector_as_python_list("time_ms_existing_keys_min",
                              time_ms_existing_keys_min);
  print_vector_as_python_list("time_ms_existing_keys_max",
                              time_ms_existing_keys_max);
  print_vector_as_python_list("time_ms_non_existing_keys",
                              time_ms_non_existing_keys);
  print_vector_as_python_list("time_ms_non_existing_keys_min",
                              time_ms_non_existing_keys_min);
  print_vector_as_python_list("time_ms_non_existing_keys_max",
                              time_ms_non_existing_keys_max);
  print_vector_as_python_list("time_ms_existing_keys_stddev",
                              time_ms_existing_keys_stddev);
  print_vector_as_python_list("time_ms_non_existing_keys_stddev",
                              time_ms_non_existing_keys_stddev);
}

#endif // UTIL_H