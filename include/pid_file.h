#pragma once
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

//Contains helpers for PID files
//allows rtctl to find daemon
struct PidFile {
  std::string path;
  bool active = false;

  //attempts to make a pid file
  bool create(const std::string& p) {
    path = p;
    FILE* f = std::fopen(path.c_str(), "w");
    if (!f) return false;

    std::fprintf(f, "%d\n", (int)getpid());
    std::fflush(f);
    std::fclose(f);

    active = true;
    return true;
  }

  void remove() {
    if (!active) return;
    ::unlink(path.c_str());
    active = false;
  }

  ~PidFile() { remove(); }
};
