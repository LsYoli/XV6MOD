#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  volatile uint x = 0;
  for(int i = 0; i < 100000000; i++)
    x += i;
  printf("cpu %d\n", x);
  return 0;
}
