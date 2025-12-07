#include "types.h"

int
mlfq_clamp(int level)
{
  if(level < 0)
    return 0;
  if(level > 2)
    return 2;
  return level;
}
