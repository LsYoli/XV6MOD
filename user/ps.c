#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct uproc procs[NPROC];
  int n = ps((struct uproc *)procs, NPROC);

  if(n < 0){
    fprintf(2, "ps: error\n");
    exit(1);
  }

  printf("PID  PRIORITY  QUEUE  T0  T1  T2  STATE\n");
  for(int i = 0; i < n; i++){
    printf("%d        %d      %d   %lu  %lu  %lu  %s\n",
           procs[i].pid, procs[i].priority, procs[i].queue,
           procs[i].ticks[0], procs[i].ticks[1], procs[i].ticks[2],
           procs[i].state);
  }
  exit(0);
}
