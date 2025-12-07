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

struct block {
  struct block *next;
  char *start;
  int pages;
};

struct {
  struct spinlock lock;
  struct block *freelist;
} kmem;

#define MAXBLOCKS ((PHYSTOP - (uint64)end) / PGSIZE)
static struct block blocks[MAXBLOCKS];
static int block_count = 0;
static struct block *meta_free = 0;

static struct block*
alloc_block(char *start, int pages)
{
  struct block *b;
  if(meta_free){
    b = meta_free;
    meta_free = meta_free->next;
  } else {
    if(block_count >= MAXBLOCKS)
      return 0;
    b = &blocks[block_count++];
  }
  b->start = start;
  b->pages = pages;
  b->next = 0;
  return b;
}

static void
free_block_meta(struct block *b)
{
  b->next = meta_free;
  meta_free = b;
}

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
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  memset(pa, 1, PGSIZE);

  acquire(&kmem.lock);

  struct block *prev = 0, *curr = kmem.freelist;
  struct block *newb = alloc_block((char*)pa, 1);
  if(newb == 0)
    panic("kfree: out of metadata");

  while(curr && curr->start < newb->start){
    prev = curr;
    curr = curr->next;
  }
  newb->next = curr;
  if(prev)
    prev->next = newb;
  else
    kmem.freelist = newb;

  if(newb->next && newb->start + newb->pages * PGSIZE == newb->next->start){
    newb->pages += newb->next->pages;
    struct block *next = newb->next;
    newb->next = newb->next->next;
    free_block_meta(next);
  }
  if(prev && prev->start + prev->pages * PGSIZE == newb->start){
    prev->pages += newb->pages;
    prev->next = newb->next;
    free_block_meta(newb);
  }

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct block *best = 0, *bestprev = 0;
  struct block *prev = 0, *curr;

  acquire(&kmem.lock);
  for(curr = kmem.freelist; curr; prev = curr, curr = curr->next){
    if(curr->pages >= 1 && (!best || curr->pages < best->pages)){
      best = curr;
      bestprev = prev;
    }
  }

  if(best == 0){
    release(&kmem.lock);
    return 0;
  }

  char *pa = best->start;
  best->start += PGSIZE;
  best->pages -= 1;
  if(best->pages == 0){
    if(bestprev)
      bestprev->next = best->next;
    else
      kmem.freelist = best->next;
    free_block_meta(best);
  }

  release(&kmem.lock);

  memset((char*)pa, 5, PGSIZE);
  return (void*)pa;
}
