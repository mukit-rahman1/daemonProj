#pragma once
#include <cstdio>
#include <string>
#include <cerrno> //err numbers
#include <cstring> //std::strerror. err msgs
#include <sys/stat.h> //for chmod
#include "rt_config.h"
#include "stats.h"

//helper function to print stats into .stats file
static inline void write_stats_out(const Config& cfg, const Stats& st) {
  if (cfg.stats_out.empty()) return;

  const std::string tmp = cfg.stats_out + ".tmp";
  FILE* f = std::fopen(tmp.c_str(), "w");
  if (!f) {
    std::fprintf(stderr, "rtmonitord: stats_out fopen('%s') failed: %s\n",
                 tmp.c_str(), std::strerror(errno));
    return;
  }

  const double miss_rate = (st.samples > 0)
    ? (100.0 * (double)st.misses / (double)st.samples)
    : 0.0;

  std::fprintf(f,
    "samples=%llu min_ns=%lld avg_ns=%.1Lf max_ns=%lld misses=%llu miss_rate_pct=%.6f\n",
    (unsigned long long)st.samples,
    (long long)st.min_or0(),
    st.avg(),
    (long long)st.max_or0(),
    (unsigned long long)st.misses,
    miss_rate
  );
  std::fflush(f);
  std::fclose(f);

  ::chmod(tmp.c_str(), 0644); //owner can read and write. Everyone else can read, no one else can write

  //replace and update old file
  if (::rename(tmp.c_str(), cfg.stats_out.c_str()) != 0) {
    std::fprintf(stderr, "rtmonitord: rename('%s'->'%s') failed: %s\n",
                 tmp.c_str(), cfg.stats_out.c_str(), std::strerror(errno));
  }
}
