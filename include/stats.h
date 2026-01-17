#pragma once
#include <cstdint>
#include <limits>

struct Stats {
  uint64_t samples = 0;
  uint64_t misses = 0;
  int64_t min_jitter_ns = std::numeric_limits<int64_t>::max();
  int64_t max_jitter_ns = std::numeric_limits<int64_t>::min();
  long double sum_jitter_ns = 0.0L;

  void add(int64_t jitter_ns, int64_t miss_threshold_ns) {
    samples++;
    sum_jitter_ns += (long double)jitter_ns;
    if (jitter_ns < min_jitter_ns) min_jitter_ns = jitter_ns;
    if (jitter_ns > max_jitter_ns) max_jitter_ns = jitter_ns;
    if (jitter_ns > miss_threshold_ns) misses++;
  }

  long double avg() const {
    return samples ? (sum_jitter_ns / (long double)samples) : 0.0L;
  }

  int64_t min_or0() const {
    return (min_jitter_ns == std::numeric_limits<int64_t>::max()) ? 0 : min_jitter_ns;
  }

  int64_t max_or0() const {
    return (max_jitter_ns == std::numeric_limits<int64_t>::min()) ? 0 : max_jitter_ns;
  }

  void reset() { *this = Stats{}; }
};
