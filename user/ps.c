#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/proc.h"
#include "kernel/procinfo.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
  struct uproc table[NPROC];
  int n = ps(table, NPROC);
  if(n < 0){
    fprintf(2, "ps: failed\n");
    exit(1);
  }

  printf("PID  PRIORITY  QUEUE  T0  T1  T2  STATE\n");
  for(int i = 0; i < n; i++){
    char *state;
    switch(table[i].state){
    case UNUSED: state = "UNUSED"; break;
    case USED: state = "USED"; break;
    case SLEEPING: state = "SLEEP"; break;
    case RUNNABLE: state = "RUNNABLE"; break;
    case RUNNING: state = "RUNNING"; break;
    case ZOMBIE: state = "ZOMBIE"; break;
    default: state = "?"; break;
    }
    printf("%d    %d         %d     %d   %d   %d   %s\n",
           table[i].pid,
           table[i].priority,
           table[i].queue,
           table[i].ticks[0],
           table[i].ticks[1],
           table[i].ticks[2],
           state);
  }
  exit(0);
}
