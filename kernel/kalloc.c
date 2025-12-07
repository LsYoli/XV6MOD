// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
  int npages; // size of the block in pages
};

struct {
  struct spinlock lock;
  struct run *freelist; // sorted by address
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  if(p + PGSIZE > (char*)pa_end)
    return;

  struct run *block = (struct run*)p;
  block->npages = ((uint64)pa_end - (uint64)p) / PGSIZE;
  block->next = 0;

  acquire(&kmem.lock);
  kmem.freelist = block;
  release(&kmem.lock);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  struct run *block = (struct run*)pa;
  block->npages = 1;
  block->next = 0;

  acquire(&kmem.lock);

  struct run *prev = 0;
  struct run *curr = kmem.freelist;
  while(curr && (uint64)curr < (uint64)block){
    prev = curr;
    curr = curr->next;
  }

  block->next = curr;
  if(prev)
    prev->next = block;
  else
    kmem.freelist = block;

  // Coalesce with next
  if(block->next && (char*)block + block->npages * PGSIZE == (char*)block->next){
    block->npages += block->next->npages;
    block->next = block->next->next;
  }

  // Coalesce with prev
  if(prev && (char*)prev + prev->npages * PGSIZE == (char*)block){
    prev->npages += block->npages;
    prev->next = block->next;
  }

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *best = 0, *bestprev = 0;
  struct run *prev = 0;
  acquire(&kmem.lock);
  for(struct run *r = kmem.freelist; r; prev = r, r = r->next){
    if(r->npages >= 1 && (!best || r->npages < best->npages)){
      best = r;
      bestprev = prev;
      if(best->npages == 1)
        break;
    }
  }

  if(best == 0){
    release(&kmem.lock);
    return 0;
  }

  void *pa = (void*)best;
  if(best->npages == 1){
    if(bestprev)
      bestprev->next = best->next;
    else
      kmem.freelist = best->next;
  } else {
    struct run *newblock = (struct run*)((char*)best + PGSIZE);
    newblock->npages = best->npages - 1;
    newblock->next = best->next;
    if(bestprev)
      bestprev->next = newblock;
    else
      kmem.freelist = newblock;
  }

  release(&kmem.lock);

  memset(pa, 5, PGSIZE);
  return pa;
}
