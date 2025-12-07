#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  for(int i = 0; i < 50; i++){
    printf("io %d\n", i);
    sleep(5);
  }
  return 0;
}
