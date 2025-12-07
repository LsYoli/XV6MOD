#include "user.h"

int
main(void)
{
  volatile int x = 0;
  for(;;){
    for(int i = 0; i < 500000; i++)
      x += i;
    write(1, "\n", 1);
    pause(5);
  }
  return x;
}
