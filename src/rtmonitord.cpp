#include <csignal>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cinttypes>
#include <ctime>

#include "rt_config.hpp"
#include "time_utils.hpp"
#include "stats.hpp"
#include "rt_sched.hpp"

static volatile std::sig_atomic_t g_stop = 0;
static void on_sig(int){ g_stop = 1; }

/*
prevent page swapping
set sched policy
starting time stamp

While loop (until ctrl c detect)
monotonic clock set sleep till next wakup
measure
update
print
*/

int main(int argc, char** argv) {
  std::signal(SIGINT, on_sig);
  std::signal(SIGTERM, on_sig);

  Config cfg = parse_args(argc, argv);

  //prevent pages from getting swapped
  try_mlockall(cfg.mlock);
  //set scheduler policy(algorithm) and prio
  try_set_realtime(cfg.policy, cfg.prio);

  const int64_t period_ns = 1000000000LL / (int64_t)cfg.rate_hz;  //converts to nano secs
  const int64_t report_every_ns = (int64_t)cfg.report_every_ms * 1000000LL;

  //starting time stamp
  timespec now{};
  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    std::fprintf(stderr, "rtmond: clock_gettime failed: %s\n", std::strerror(errno));
    return 1;
  }

  timespec next = add_ns(now, period_ns); // schedule first tick slightly ahead
  int64_t last_report_ns = ts_to_ns(now);

  Stats st{};

  std::printf("rtmond: rate=%dHz period=%" PRId64 "ns miss=%" PRId64 "ns report=%dms policy=%s prio=%d mlock=%d\n",
              cfg.rate_hz, period_ns, cfg.miss_threshold_ns, cfg.report_every_ms,
              policy_name(cfg.policy), cfg.prio, cfg.mlock ? 1 : 0);
  std::fflush(stdout);


  while (!g_stop) {
    //monotonic clock. sleep until next scheduled wake up
    int rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
    if (rc != 0 && rc != EINTR) {
      std::fprintf(stderr, "rtmond: clock_nanosleep failed: %s\n", std::strerror(rc));
      return 1;
    }
    if (g_stop) break;

    //measure
    timespec actual{};
    if (clock_gettime(CLOCK_MONOTONIC, &actual) != 0) {
      std::fprintf(stderr, "rtmond: clock_gettime failed: %s\n", std::strerror(errno));
      return 1;
    }

    //update
    const int64_t jitter_ns = ts_to_ns(actual) - ts_to_ns(next);
    st.add(jitter_ns, cfg.miss_threshold_ns);

    //report every ns. time spec to nano
    const int64_t cur_ns = ts_to_ns(actual);
    if (cur_ns - last_report_ns >= report_every_ns) {
      std::printf("rtmond: samples=%" PRIu64 " jitter_ns(min/avg/max)=(%" PRId64 "/%.1Lf/%" PRId64 ") misses=%" PRIu64 "\n",
                  st.samples, st.min_or0(), st.avg(), st.max_or0(), st.misses);
      std::fflush(stdout);
      last_report_ns = cur_ns;
    }

    next = add_ns(next, period_ns);
  }

  //print stats
  std::printf("rtmond: STOP samples=%" PRIu64 " jitter_ns(min/avg/max)=(%" PRId64 "/%.1Lf/%" PRId64 ") misses=%" PRIu64 "\n",
              st.samples, st.min_or0(), st.avg(), st.max_or0(), st.misses);
  std::fflush(stdout);
  return 0;
}
