#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uprocs.h"
#include "user/user.h"

static const char *states[] = {
  [U_UNUSED]   "UNUSED",
  [U_USED]     "USED",
  [U_SLEEPING] "SLEEPING",
  [U_RUNNABLE] "RUNNABLE",
  [U_RUNNING]  "RUNNING",
  [U_ZOMBIE]   "ZOMBIE"
};

int
main(int argc, char *argv[])
{
  struct uproc procs[NPROC];
  int n = ps(procs, NPROC);
  if(n < 0){
    printf("ps: error\n");
    exit(1);
  }

  printf("PID  PRIORITY  QUEUE  T0  T1  T2  STATE\n");
  for(int i = 0; i < n; i++){
    struct uproc *p = &procs[i];
    const char *state = (p->state >= 0 && p->state < sizeof(states)/sizeof(states[0]) && states[p->state])
                        ? states[p->state] : "?";
    printf("%d    %d         %d     %d   %d   %d   %s\n",
           p->pid, p->priority, p->queue,
           (int)p->ticks[0], (int)p->ticks[1], (int)p->ticks[2], state);
  }
  exit(0);
}
