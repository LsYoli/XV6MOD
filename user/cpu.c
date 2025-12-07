#include "user.h"

int
main(void)
{
  volatile int x = 1;
  for(;;)
    x = x * 3 + 1;
  return x;
}
