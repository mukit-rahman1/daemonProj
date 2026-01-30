#include <libnotify/notify.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

/*
Open and read stats file
Create deskpop up
Keep extracting all fields of stats file and update
repeat every 1 sec
*/

static void usage(const char* p) {
  std::fprintf(stderr,
    "Usage: %s [--stats PATH] [--period-ms N]\n"
    "Default stats: /run/rtmonitord/rtmonitord.stats\n"
    "Default period: 1000ms\n"
    "Stop: Ctrl+C\n", p);
}

//helper func to read line stats file
static bool read_line(const std::string& path, std::string& out) {
  FILE* f = std::fopen(path.c_str(), "r");
  if (!f) return false;
  char buf[512];
  if (!std::fgets(buf, sizeof(buf), f)) { std::fclose(f); return false; }
  std::fclose(f);
  out = buf;
  // strip newline
  while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
  return true;
}

//Convert ns to readable ns/us/ms
static std::string fmt_ns(long long ns) {
  double v = (double)ns;
  char b[64];
  if (std::llabs(ns) < 1000) std::snprintf(b, sizeof(b), "%.0f ns", v);
  else if (std::llabs(ns) < 1000000) std::snprintf(b, sizeof(b), "%.1f us", v/1000.0);
  else std::snprintf(b, sizeof(b), "%.3f ms", v/1000000.0);
  return b;
}

int main(int argc, char** argv) {
  std::string stats_path = "/run/rtmonitord/rtmonitord.stats";
  int period_ms = 1000;

  for (int i = 1; i < argc; i++) {
    if (!std::strcmp(argv[i], "--help") || !std::strcmp(argv[i], "-h")) {
      usage(argv[0]); return 0;
    } else if (!std::strcmp(argv[i], "--stats")) {
      if (i + 1 >= argc) { usage(argv[0]); return 2; }
      stats_path = argv[++i];
    } else if (!std::strcmp(argv[i], "--period-ms")) {
      if (i + 1 >= argc) { usage(argv[0]); return 2; }
      period_ms = std::atoi(argv[++i]);
      if (period_ms < 100) period_ms = 100;
    } else {
      usage(argv[0]); return 2;
    }
  }

  if (!notify_init("Jitter Monitor")) {
    std::fprintf(stderr, "rtnotify: notify_init failed\n");
    return 1;
  }

  //create the sticky notification
  NotifyNotification* n = notify_notification_new("Jitter Monitor", "Waiting for stats…", nullptr);
  notify_notification_set_timeout(n, NOTIFY_EXPIRES_NEVER);

  // Keep updating the same notification
  while (true) {
    std::string line;
    if (!read_line(stats_path, line)) {
      std::string body = "Waiting: " + stats_path;
      notify_notification_update(n, "Jitter Monitor", body.c_str(), nullptr);
      notify_notification_show(n, nullptr);
      usleep((useconds_t)period_ms * 1000);
      continue;
    }

    unsigned long long samples=0, misses=0;
    long long min_ns=0, max_ns=0;
    long double avg_ns=0.0;
    double miss_rate=0.0;

    // expected format from stats_out.h
    int ok = std::sscanf(line.c_str(),
      "samples=%llu min_ns=%lld avg_ns=%Lf max_ns=%lld misses=%llu miss_rate_pct=%lf",
      &samples, &min_ns, &avg_ns, &max_ns, &misses, &miss_rate);

    //if successfully extracted all 6 fields
    if (ok == 6) {
      std::string body =
        "avg: " + fmt_ns((long long)avg_ns) +
        "   max: " + fmt_ns(max_ns) +
        "\nmiss%: " + std::to_string(miss_rate).substr(0, 6) +
        "   samples: " + std::to_string(samples) +
        "   misses: " + std::to_string(misses);

      //keep updating
      notify_notification_update(n, "Jitter Monitor", body.c_str(), nullptr);
      notify_notification_show(n, nullptr);
    } else {
      notify_notification_update(n, "Jitter Monitor", line.c_str(), nullptr);
      notify_notification_show(n, nullptr);
    }
    //repeat after 1 sec
    usleep((useconds_t)period_ms * 1000);
  }
  return 0;
}
