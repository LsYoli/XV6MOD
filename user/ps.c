#include "user.h"

static char *states[] = {"unused", "used", "sleep", "runbl", "run", "zomb"};

int
main(void)
{
  struct pinfo info[64];
  int n = psinfo(info, 64);

  printf("PID\tSTATE\tPRIO\tQUEUE\tTICKS\n");
  for(int i = 0; i < n; i++)
    printf("%d\t%s\t%d\t%d\t%d\n", info[i].pid,
           states[info[i].state], info[i].priority,
           info[i].queue, info[i].ticks);
  exit(0);
}
