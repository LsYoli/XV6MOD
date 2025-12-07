#include "types.h"
#include "spinlock.h"
#include "defs.h"

static struct {
  struct spinlock lock;
  int last_event;
  int inited;
} iostat;

static void
io_init(void)
{
  if(iostat.inited)
    return;
  initlock(&iostat.lock, "iostat");
  iostat.inited = 1;
}

void
io_note_event(int tag)
{
  io_init();
  acquire(&iostat.lock);
  iostat.last_event = tag;
  release(&iostat.lock);
}

int
io_last_event(void)
{
  int v;
  io_init();
  acquire(&iostat.lock);
  v = iostat.last_event;
  release(&iostat.lock);
  return v;
}
