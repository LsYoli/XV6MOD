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
  uint64 size; // size in pages
};

struct {
  struct spinlock lock;
  struct run *freelist;
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
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  r->size = 1;

  acquire(&kmem.lock);

  // Insert into sorted free list.
  struct run *prev = 0, *curr = kmem.freelist;
  while(curr && (uint64)curr < (uint64)r){
    prev = curr;
    curr = curr->next;
  }
  r->next = curr;
  if(prev)
    prev->next = r;
  else
    kmem.freelist = r;

  // Coalesce with next block.
  if(r->next && (char*)r + r->size * PGSIZE == (char*)r->next){
    r->size += r->next->size;
    r->next = r->next->next;
  }

  // Coalesce with previous block.
  if(prev && (char*)prev + prev->size * PGSIZE == (char*)r){
    prev->size += r->size;
    prev->next = r->next;
  }

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r = 0, *prev = 0, *best = 0, *bestprev = 0;

  acquire(&kmem.lock);
  r = kmem.freelist;

  // Find best-fit block (smallest that still fits one page).
  while(r) {
    if(r->size >= 1 && (best == 0 || r->size < best->size)) {
      best = r;
      bestprev = prev;
      if(r->size == 1)
        break;
    }
    prev = r;
    r = r->next;
  }

  if(best == 0){
    release(&kmem.lock);
    return 0;
  }

  void *pa = (void*)best;
  if(best->size == 1){
    // Remove block from list.
    if(bestprev)
      bestprev->next = best->next;
    else
      kmem.freelist = best->next;
  } else {
    // Split block: allocate first page and keep remainder.
    struct run *newblock = (struct run*)((char*)best + PGSIZE);
    newblock->size = best->size - 1;
    newblock->next = best->next;
    if(bestprev)
      bestprev->next = newblock;
    else
      kmem.freelist = newblock;
  }

  release(&kmem.lock);

  memset(pa, 5, PGSIZE); // fill with junk
  return pa;
}
