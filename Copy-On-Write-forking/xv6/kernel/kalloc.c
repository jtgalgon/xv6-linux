// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;

  /*
  P5 changes
  */
  uint free_pages; //track free pages
  uint ref_cnt[PHYSTOP / PGSIZE]; //track reference count
} kmem;

extern char end[]; // first address after kernel loaded from ELF file

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;
  initlock(&kmem.lock, "kmem");
  p = (char*)PGROUNDUP((uint)end);
  
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE){
    kmem.ref_cnt[(uint)p / PGSIZE] = 1;
    kfree(p);
  }
  

}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || (uint)v >= PHYSTOP) 
    panic("kfree");

  if(kmem.ref_cnt[(uint)v/PGSIZE] <= 1){
    // Fill with junk to catch dangling refs.
    memset(v, 1, PGSIZE);
    acquire(&kmem.lock);
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }else{
    decrementrefcount((uint)v);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.ref_cnt[(uint)r / PGSIZE] = 1;
  }
  release(&kmem.lock);
  return (char*)r;
}

//returns the total number of free pages in kmem
int
getFreePagesCount(void)
{
  int i = 0;
  acquire(&kmem.lock);
  struct run *r = kmem.freelist;
  while(r != NULL){
    i++;
    r = r->next;
  }
  kmem.free_pages = i;
  release(&kmem.lock);
  return i;
}

void
incrementrefcount(uint index)
{
  acquire(&kmem.lock);
  index = index / PGSIZE;
  kmem.ref_cnt[index]+= 1;
  release(&kmem.lock);
}

void
decrementrefcount(uint index)
{
  acquire(&kmem.lock);
  index = index / PGSIZE;
  kmem.ref_cnt[index]-= 1;
  release(&kmem.lock);
}

int
getrefcount(uint index)
{
 
  return kmem.ref_cnt[index/PGSIZE];
}


