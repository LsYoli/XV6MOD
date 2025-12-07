#include "user.h"

int
main(void)
{
  char c = 'A';
  for(;;){
    write(1, &c, 1);
    pause(10);
  }
  return 0;
}
