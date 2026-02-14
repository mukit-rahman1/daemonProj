#pragma once
#include <cerrno> //get err code from posix
#include <cstring> //help explain err
#include <cstdio> 
#include <pthread.h> //posix
#include <sched.h> //sched policies
#include <sys/mman.h> //locking pages

//inline prevents multiple definition link errors

enum class RtPolicy { Other, Fifo, Rr };

//helper for OS call
static inline int to_native_policy(RtPolicy p) {
  switch (p) {
    case RtPolicy::Fifo: return SCHED_FIFO; //closed scoped enum for good practice
    case RtPolicy::Rr:   return SCHED_RR;
    case RtPolicy::Other:
    default:             return SCHED_OTHER;
  }
}
//helper for logging
static inline const char* policy_name(RtPolicy p) {
  switch (p) {
    case RtPolicy::Fifo: return "SCHED_FIFO";
    case RtPolicy::Rr:   return "SCHED_RR";
    case RtPolicy::Other:
    default:             return "SCHED_OTHER";
  }
}

//prevent page swapping for just this daemon
static inline void try_mlockall(bool enable) {
  if (!enable) return;
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) { //MCL bit flags. Check for err
    std::fprintf(stderr, "rtmond: mlockall failed: %s\n", std::strerror(errno));
  }
}

//set the scheduler policy using POSIX threads function
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
