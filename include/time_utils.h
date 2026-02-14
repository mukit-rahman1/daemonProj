#pragma once
#include <cstdint> //int64_t
#include <ctime> //timespec 

//helpers for timespec as monotonic clock expects it

//convert timsepec to ns
static inline int64_t ts_to_ns(const timespec& ts) {
  return int64_t(ts.tv_sec) * 1000000000LL + int64_t(ts.tv_nsec);
}

//convert ns to timespec
static inline timespec ns_to_ts(int64_t ns) {
  timespec ts{};
  ts.tv_sec = time_t(ns / 1000000000LL);
  ts.tv_nsec = long(ns % 1000000000LL);
  return ts;
}

//add ns to the timespec
static inline timespec add_ns(const timespec& a, int64_t add) {
  return ns_to_ts(ts_to_ns(a) + add);
}
