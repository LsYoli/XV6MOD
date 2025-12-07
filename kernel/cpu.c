#include "types.h"
#include "spinlock.h"
#include "defs.h"

static struct {
  struct spinlock lock;
  uint last;
  int inited;
} cstat;

static void
cpu_init(void)
{
  if(cstat.inited)
    return;
  initlock(&cstat.lock, "cstat");
  cstat.inited = 1;
}

void
cpu_track_burst(uint ticks)
{
  cpu_init();
  acquire(&cstat.lock);
  cstat.last = ticks;
  release(&cstat.lock);
}

uint
cpu_last_burst(void)
{
  uint v;
  cpu_init();
  acquire(&cstat.lock);
  v = cstat.last;
  release(&cstat.lock);
  return v;
}
