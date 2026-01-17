#pragma once
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>

enum class RtPolicy { Other, Fifo, Rr };

static inline int to_native_policy(RtPolicy p) {
  switch (p) {
    case RtPolicy::Fifo: return SCHED_FIFO;
    case RtPolicy::Rr:   return SCHED_RR;
    case RtPolicy::Other:
    default:             return SCHED_OTHER;
  }
}

static inline const char* policy_name(RtPolicy p) {
  switch (p) {
    case RtPolicy::Fifo: return "SCHED_FIFO";
    case RtPolicy::Rr:   return "SCHED_RR";
    case RtPolicy::Other:
    default:             return "SCHED_OTHER";
  }
}

//prevent page swapping
static inline void try_mlockall(bool enable) {
  if (!enable) return;
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    std::fprintf(stderr, "rtmond: mlockall failed: %s\n", std::strerror(errno));
  }
}

//set the scheduler policy
static inline void try_set_realtime(RtPolicy policy, int prio) {
  if (policy == RtPolicy::Other) return;

  sched_param sp{};
  sp.sched_priority = prio;

  int rc = pthread_setschedparam(pthread_self(), to_native_policy(policy), &sp);
  if (rc != 0) {
    std::fprintf(stderr,
      "rtmond: pthread_setschedparam(%s, prio=%d) failed: %s\n"
      "        Hint: run as root or grant CAP_SYS_NICE.\n",
      policy_name(policy), prio, std::strerror(rc));
  }
}
