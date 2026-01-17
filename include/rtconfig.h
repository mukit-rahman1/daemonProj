#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "rt_sched.hpp"

//runtime settings
struct Config {
  int rate_hz = 100;
  int report_every_ms = 1000;
  int64_t miss_threshold_ns = 1000 * 1000; // 1ms
  RtPolicy policy = RtPolicy::Other;
  int prio = 80;      // used for FIFO/RR
  bool mlock = false; // mlockall
};

static inline bool streq(const char* a, const char* b) { return std::strcmp(a,b)==0; }

static inline void usage(const char* prog) {
  std::fprintf(stderr,
    "Usage: %s [--rate HZ] [--report-ms MS] [--miss-us USEC]\n"
    "          [--policy other|fifo|rr] [--prio N] [--mlock]\n"
    "\n"
    "Examples:\n"
    "  %s --rate 1000 --policy rr --prio 80 --mlock\n"
    "  %s --rate 200 --policy fifo --prio 90\n",
    prog, prog, prog);
}

static inline bool parse_int(const char* s, int& out) {
  char* end=nullptr; long v=std::strtol(s,&end,10);
  if (!s || *s=='\0' || (end && *end!='\0')) return false;
  out = (int)v; return true;
}

static inline bool parse_i64(const char* s, int64_t& out) {
  char* end=nullptr; long long v=std::strtoll(s,&end,10);
  if (!s || *s=='\0' || (end && *end!='\0')) return false;
  out = (int64_t)v; return true;
}

static inline RtPolicy parse_policy(const char* s) {
  if (!s) return RtPolicy::Other;
  if (std::strcmp(s,"fifo")==0) return RtPolicy::Fifo;
  if (std::strcmp(s,"rr")==0)   return RtPolicy::Rr;
  return RtPolicy::Other;
}

static inline Config parse_args(int argc, char** argv) {
  Config c{};
  for (int i=1;i<argc;i++) {
    if (streq(argv[i],"--help") || streq(argv[i],"-h")) { usage(argv[0]); std::exit(0); }

    else if (streq(argv[i],"--rate")) {
      if (i+1>=argc) { usage(argv[0]); std::exit(2); }
      int v=0; if(!parse_int(argv[++i],v) || v<=0) { usage(argv[0]); std::exit(2); }
      c.rate_hz=v;

    } else if (streq(argv[i],"--report-ms")) {
      if (i+1>=argc) { usage(argv[0]); std::exit(2); }
      int v=0; if(!parse_int(argv[++i],v) || v<=0) { usage(argv[0]); std::exit(2); }
      c.report_every_ms=v;

    } else if (streq(argv[i],"--miss-us")) {
      if (i+1>=argc) { usage(argv[0]); std::exit(2); }
      int64_t us=0; if(!parse_i64(argv[++i],us) || us<0) { usage(argv[0]); std::exit(2); }
      c.miss_threshold_ns = us * 1000LL;

    } else if (streq(argv[i],"--policy")) {
      if (i+1>=argc) { usage(argv[0]); std::exit(2); }
      c.policy = parse_policy(argv[++i]);

    } else if (streq(argv[i],"--prio")) {
      if (i+1>=argc) { usage(argv[0]); std::exit(2); }
      int v=0; if(!parse_int(argv[++i],v) || v<1 || v>99) { usage(argv[0]); std::exit(2); }
      c.prio=v;

    } else if (streq(argv[i],"--mlock")) {
      c.mlock = true;

    } else {
      std::fprintf(stderr,"Unknown arg: %s\n", argv[i]);
      usage(argv[0]); std::exit(2);
    }
  }
  return c;
}
