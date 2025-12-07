#ifndef UPROCS_H
#define UPROCS_H

#include "types.h"

// Lightweight process snapshot for user-space reporting.
enum ustate { U_UNUSED = 0, U_USED, U_SLEEPING, U_RUNNABLE, U_RUNNING, U_ZOMBIE };

struct uproc {
  int pid;
  int priority;
  int queue;
  uint64 ticks[3];
  int state;
};

#endif // UPROCS_H
