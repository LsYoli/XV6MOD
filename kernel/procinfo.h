#ifndef PROCINFO_H
#define PROCINFO_H

struct uproc {
  int pid;
  int priority;
  int queue;
  int ticks[3];
  int state;
};

#endif
