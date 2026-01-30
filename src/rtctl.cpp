#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>


//Mini helper CLI. (real-time controller)
/*
get args

possible uses
create interrupts to be used by daemon
print necessary msgs for each flag
*/

//helper func for -h or -help
static void usage(const char* prog) {
  std::fprintf(stderr,
    "Usage: %s [--pidfile PATH] <reset|status|stop>\n"
    "\n"
    "  reset   -> SIGUSR1 (clear stats)\n"
    "  status  -> SIGUSR2 (daemon prints snapshot to journald/stdout)\n"
    "  stop    -> SIGTERM (graceful exit)\n"
    "\n"
    "Default pidfile: /run/rtmonitord/rtmonitord.pid\n",
    prog
  );
}

static pid_t read_pidfile(const std::string& path) {
  FILE* f = std::fopen(path.c_str(), "r");
  if (!f) return -1;
  long pid = -1;
  if (std::fscanf(f, "%ld", &pid) != 1) pid = -1;
  std::fclose(f);
  return (pid_t)pid;
}

int main(int argc, char** argv) {
  std::string pidfile = "/run/rtmonitord/rtmonitord.pid";
  std::string cmd;

  //get args
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
      usage(argv[0]); return 0;
    } else if (std::strcmp(argv[i], "--pidfile") == 0) {
      if (i + 1 >= argc) { usage(argv[0]); return 2; }
      pidfile = argv[++i];
    } else {
      cmd = argv[i];
    }
  }

  if (cmd.empty()) { usage(argv[0]); return 2; }

  //open pidfile to find daemon
  pid_t pid = read_pidfile(pidfile);
  if (pid <= 0) {
    std::fprintf(stderr, "rtctl: failed to read pid from %s: %s\n",
                 pidfile.c_str(), std::strerror(errno));
    return 1;
  }

  //create interrupt signals to be used by the daemon
  int sig = 0;
  if (cmd == "reset") sig = SIGUSR1;
  else if (cmd == "status") sig = SIGUSR2;
  else if (cmd == "stop") sig = SIGTERM;
  else { usage(argv[0]); return 2; }

  if (::kill(pid, sig) != 0) {
    std::fprintf(stderr, "rtctl: kill(%d, %d) failed: %s\n", (int)pid, sig, std::strerror(errno));
    return 1;
  }

  std::printf("rtctl: sent %s to pid %d (pidfile=%s)\n", cmd.c_str(), (int)pid, pidfile.c_str());
  if (cmd == "status") {
    std::printf("rtctl: check output via journald/systemd later, or daemon stdout.\n");
  }
  return 0;
}
