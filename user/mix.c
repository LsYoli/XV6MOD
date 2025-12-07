#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  volatile uint x = 1;
  for(int i = 0; i < 20; i++){
    for(int j = 0; j < 500000; j++)
      x = x * 3 + j;
    printf("mix %d\n", i);
    sleep(2);
  }
  printf("mix %d\n", x);
  return 0;
}
