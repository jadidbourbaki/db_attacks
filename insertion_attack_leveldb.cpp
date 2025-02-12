#include "util.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <cassert>
#include <iostream>

// -----------------------------------------------------------------------
// This code has been taken from LevelDB version 1.23 in util/hash.cc,
// util/coding.h, include/leveldb/slice.h, and util/bloom.cc
// -----------------------------------------------------------------------

// -----------------------
// include/leveldb/slice.h
// -----------------------

class Slice {
public:
  // Create an empty slice.
  Slice() : data_(""), size_(0) {}

  // Create a slice that refers to d[0,n-1].
  Slice(const char *d, size_t n) : data_(d), size_(n) {}

  // Create a slice that refers to the contents of "s"
  Slice(const std::string &s) : data_(s.data()), size_(s.size()) {}

  // Create a slice that refers to s[0,strlen(s)-1]
  Slice(const char *s) : data_(s), size_(strlen(s)) {}

  // Intentionally copyable.
  Slice(const Slice &) = default;
  Slice &operator=(const Slice &) = default;

  // Return a pointer to the beginning of the referenced data
  const char *data() const { return data_; }

  // Return the length (in bytes) of the referenced data
  size_t size() const { return size_; }

  // Return true iff the length of the referenced data is zero
  bool empty() const { return size_ == 0; }

  const char *begin() const { return data(); }
  const char *end() const { return data() + size(); }

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  // Change this slice to refer to an empty array
  void clear() {
    data_ = "";
    size_ = 0;
  }

  // Drop the first "n" bytes from this slice.
  void remove_prefix(size_t n) {
    assert(n <= size());
    data_ += n;
    size_ -= n;
  }

  // Return a string that contains the copy of the referenced data.
  std::string ToString() const { return std::string(data_, size_); }

  // Three-way comparison.  Returns value:
  //   <  0 iff "*this" <  "b",
  //   == 0 iff "*this" == "b",
  //   >  0 iff "*this" >  "b"
  int compare(const Slice &b) const;

  // Return true iff "x" is a prefix of "*this"
  bool starts_with(const Slice &x) const {
    return ((size_ >= x.size_) && (memcmp(data_, x.data_, x.size_) == 0));
  }

private:
  const char *data_;
  size_t size_;
};

inline bool operator==(const Slice &x, const Slice &y) {
  return ((x.size() == y.size()) &&
          (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice &x, const Slice &y) { return !(x == y); }

inline int Slice::compare(const Slice &b) const {
  const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
  int r = memcmp(data_, b.data_, min_len);
  if (r == 0) {
    if (size_ < b.size_)
      r = -1;
    else if (size_ > b.size_)
      r = +1;
  }
  return r;
}

// --------------
// util/coding.cc
// --------------
inline uint32_t DecodeFixed32(const char *ptr) {
  const uint8_t *const buffer = reinterpret_cast<const uint8_t *>(ptr);

  // Recent clang and gcc optimize this to a single mov / ldr instruction.
  return (static_cast<uint32_t>(buffer[0])) |
         (static_cast<uint32_t>(buffer[1]) << 8) |
         (static_cast<uint32_t>(buffer[2]) << 16) |
         (static_cast<uint32_t>(buffer[3]) << 24);
}

// ------------
// util/hash.cc
// ------------

// The FALLTHROUGH_INTENDED macro can be used to annotate implicit fall-through
// between switch labels. The real definition should be provided externally.
// This one is a fallback version for unsupported compilers.
#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED                                                   \
  do {                                                                         \
  } while (0)
#endif

uint32_t Hash(const char *data, size_t n, uint32_t seed) {
  // Similar to murmur hash
  const uint32_t m = 0xc6a4a793;
  const uint32_t r = 24;
  const char *limit = data + n;
  uint32_t h = seed ^ (n * m);

  // Pick up four bytes at a time
  while (limit - data >= 4) {
    uint32_t w = DecodeFixed32(data);
    data += 4;
    h += w;
    h *= m;
    h ^= (h >> 16);
  }

  // Pick up remaining bytes
  switch (limit - data) {
  case 3:
    h += static_cast<uint8_t>(data[2]) << 16;
    FALLTHROUGH_INTENDED;
  case 2:
    h += static_cast<uint8_t>(data[1]) << 8;
    FALLTHROUGH_INTENDED;
  case 1:
    h += static_cast<uint8_t>(data[0]);
    h *= m;
    h ^= (h >> r);
    break;
  }
  return h;
}

// -------------
// util/bloom.cc
// -------------

static uint32_t BloomHash(const Slice &key) {
  return Hash(key.data(), key.size(), 0xbc9f1d34);
}

class BloomFilterPolicy {
public:
  explicit BloomFilterPolicy(int bits_per_key) : bits_per_key_(bits_per_key) {
    // We intentionally round down to reduce probing cost a little bit
    k_ = static_cast<size_t>(bits_per_key * 0.69); // 0.69 =~ ln(2)
    if (k_ < 1)
      k_ = 1;
    if (k_ > 30)
      k_ = 30;
  }

  void CreateFilter(const Slice *keys, int n, std::string *dst) const {
    // Compute bloom filter size (in both bits and bytes)
    size_t bits = n * bits_per_key_;

    // For small n, we can see a very high false positive rate.  Fix it
    // by enforcing a minimum bloom filter length.
    if (bits < 64)
      bits = 64;

    size_t bytes = (bits + 7) / 8;
    bits = bytes * 8;

    const size_t init_size = dst->size();
    dst->resize(init_size + bytes, 0);
    dst->push_back(static_cast<char>(k_)); // Remember # of probes in filter
    char *array = &(*dst)[init_size];
    for (int i = 0; i < n; i++) {
      // Use double-hashing to generate a sequence of hash values.
      // See analysis in [Kirsch,Mitzenmacher 2006].
      uint32_t h = BloomHash(keys[i]);
      const uint32_t delta = (h >> 17) | (h << 15); // Rotate right 17 bits
      for (size_t j = 0; j < k_; j++) {
        const uint32_t bitpos = h % bits;
        array[bitpos / 8] |= (1 << (bitpos % 8));
        h += delta;
      }
    }
  }

  bool KeyMayMatch(const Slice &key, const Slice &bloom_filter) const {
    const size_t len = bloom_filter.size();
    if (len < 2)
      return false;

    const char *array = bloom_filter.data();
    const size_t bits = (len - 1) * 8;

    // Use the encoded k so that we can read filters generated by
    // bloom filters created using different parameters.
    const size_t k = array[len - 1];
    if (k > 30) {
      // Reserved for potentially new encodings for short bloom filters.
      // Consider it a match.
      return true;
    }

    uint32_t h = BloomHash(key);
    const uint32_t delta = (h >> 17) | (h << 15); // Rotate right 17 bits
    for (size_t j = 0; j < k; j++) {
      const uint32_t bitpos = h % bits;
      if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0)
        return false;
      h += delta;
    }
    return true;
  }

private:
  size_t bits_per_key_;
  size_t k_;

  // adding it as a friend for convenience
  friend std::vector<std::string>
  FindMinimalSaturatingSet(BloomFilterPolicy &bloom_filter, int range_start,
                           int range_end, int &trials);

  friend const insertion_result_temp run_experiment(const int bits);
};

#define MAX_ENTRIES_CHECK 64

std::vector<std::string>
FindMinimalSaturatingSet(BloomFilterPolicy &bloom_filter, int range_start,
                         int range_end, int &trials) {
  std::unordered_set<size_t> unique_bits;
  std::vector<std::string> minimal_set;
  std::string filter_data;

  while (1) {
    trials++;

    int random_number = range_start + rand() % (range_end - range_start + 1);
    std::string key = std::to_string(random_number);
    std::string temp_filter;

    std::vector<Slice> tmp_slices;

    for (int j = 0; j < MAX_ENTRIES_CHECK; j++) {
      tmp_slices.push_back(Slice(key));
    }

    bloom_filter.CreateFilter(&tmp_slices[0], MAX_ENTRIES_CHECK, &temp_filter);

    size_t new_bits = 0;

    if (filter_data.size() == 0) {
      // first entry
      new_bits = bloom_filter.k_;
    } else {
      for (size_t b = 0; b < temp_filter.size() * 8; b++) {
        if ((temp_filter[b / 8] & (1 << (b % 8))) &&
            !(filter_data[b / 8] & (1 << (b % 8)))) {
          new_bits++;
        }
      }
    }

    if (new_bits != bloom_filter.k_) {
      continue;
    }

    minimal_set.push_back(key);

    tmp_slices.clear();
    for (int j = 0; j < MAX_ENTRIES_CHECK; j++) {
      if (j < minimal_set.size()) {
        tmp_slices.push_back(Slice(minimal_set[j]));
        continue;
      }

      tmp_slices.push_back(Slice(minimal_set[0]));
    }

    filter_data.clear();
    bloom_filter.CreateFilter(&tmp_slices[0], MAX_ENTRIES_CHECK, &filter_data);

    if (minimal_set.size() * bloom_filter.k_ >=
        bloom_filter.bits_per_key_ * MAX_ENTRIES_CHECK) {
      return minimal_set;
    }
  }

  assert(0);
  return minimal_set;
}

const insertion_result_temp run_experiment(const int bits) {
  BloomFilterPolicy bloom_filter(bits);
  int range_start = 0, range_end = NUM_KEYS;

  insertion_result_temp rtn;

  auto start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::string> result = FindMinimalSaturatingSet(
      bloom_filter, range_start, range_end, rtn.trials);
  auto end_time = std::chrono::high_resolution_clock::now();

  rtn.bits = bits;
  rtn.n_hashes = bloom_filter.k_;
  rtn.n_keys = MAX_ENTRIES_CHECK;
  rtn.minimal_set_size = result.size();
  rtn.time_ms_saturation =
      std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                            start_time)
          .count() /
      1000.0;

  return rtn;
}

void PrintInsertionResultsAsPythonList(
    const std::vector<insertion_result> &results) {
  std::cout << "results = [\n";
  for (const auto &result : results) {
    std::cout << "  {\"bits_per_key\": " << result.bits << ", ";
    std::cout << "\"hashes\": " << result.n_hashes << ", ";
    std::cout << "\"keys\": " << result.n_keys << ", ";
    std::cout << "\"trials\": [";
    for (size_t i = 0; i < result.trials.size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << result.trials[i];
    }
    std::cout << "], \"time_ms_saturation\": [";
    for (size_t i = 0; i < result.time_ms_saturation.size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << result.time_ms_saturation[i];
    }
    std::cout << "], \"minimal_set_size\": [";
    for (size_t i = 0; i < result.minimal_set_size.size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << result.minimal_set_size[i];
    }
    std::cout << "]},\n";
  }
  std::cout << "]\n";
}

int main() {
  // set the seed for reproducibility. this can be changed.
  srand(42);

  // bits per key
  auto bits_arr = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<insertion_result> results;

  for (auto bits : bits_arr) {
    std::vector<insertion_result_temp> temp_results;

    for (int i = 0; i < EXPERIMENT_ITERATION_COUNT; i++) {
      temp_results.push_back(run_experiment(bits));
      std::cout << " . " << std::flush;
    }

    std::cout << " * " << std::flush;

    results.push_back(insertion_result_from_temp(temp_results));
  }

  std::cout << std::endl;

  PrintInsertionResultsAsPythonList(results);
}
