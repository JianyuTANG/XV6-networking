#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "traps.h"
#include "debugsw.h"
#include "spinlock.h"

typedef enum shmblocktype
{
  SERIAL,PARALLEL
}shmblocktype;

#define SWAP_BUF_SIZE (PGSIZE / 4)    // Buffer size when swap.
#define MAX_SHMNODE_NUM 256    // Max number of shmnode


struct shmindex
{
  char* addrs[1024];
};

extern char data[];  // defined by kernel.ld
pde_t *kpgdir;  // for use in scheduler()
struct shmnode
{
  uint sig;//signature
  struct shmindex *addr; //page table physical address
  uint pagenum;//total page numbers in this shared area
  uint count;//how many references from processes are pointed to this sig
  shmblocktype type;//serial block or parallel block, serial default
}shmlist[MAX_SHMNODE_NUM];

// Shared memory lock
struct spinlock kshmlock;

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
  lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint)va);
  last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// There is one page table per process, plus one that's used when
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
// page protection bits prevent user code from using the kernel's
// mappings.
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap {
  void *virt;
  uint phys_start;
  uint phys_end;
  int perm;
} kmap[] = {
 { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
 { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
 { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, // kern data+memory
 { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more devices
};

// Set up kernel part of a page table.
pde_t*
setupkvm(void)
{
  pde_t *pgdir;
  struct kmap *k;

  if((pgdir = (pde_t*)kalloc()) == 0)
    return 0;
  memset(pgdir, 0, PGSIZE);
  if (P2V(PHYSTOP) > (void*)DEVSPACE)
    panic("PHYSTOP too high");
  for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
    if(mappages(pgdir, k->virt, k->phys_end - k->phys_start,
                (uint)k->phys_start, k->perm) < 0) {
      freevm(pgdir);
      return 0;
    }
  return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void
kvmalloc(void)
{
  kpgdir = setupkvm();
  switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm(void)
{
  lcr3(V2P(kpgdir));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void
switchuvm(struct proc *p)
{
  if(p == 0)
    panic("switchuvm: no process");
  if(p->kstack == 0)
    panic("switchuvm: no kstack");
  if(p->pgdir == 0)
    panic("switchuvm: no pgdir");

  pushcli();
  mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
                                sizeof(mycpu()->ts)-1, 0);
  mycpu()->gdt[SEG_TSS].s = 0;
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  mycpu()->ts.iomb = (ushort) 0xFFFF;
  ltr(SEG_TSS << 3);
  lcr3(V2P(p->pgdir));  // switch to process's address space
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem;

  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
  memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pte_t *pte;

  if((uint) addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi(ip, P2V(pa), offset+i, n) != n)
      return -1;
  }
  return 0;
}


// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm_from_kernel(pde_t *pgdir, char *addr, char *code_start, char *code_end, uint offset, uint sz) {
  uint i, pa, n;
  pte_t *pte;

  if ((uint) addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for (i = 0; i < sz; i += PGSIZE) {
    if ((pte = walkpgdir(pgdir, addr + i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if (sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if (readm(code_start, P2V(pa), offset + i, n, code_end) != n)
      return -1;
  }
  return 0;
}

// Find a usable slot and record it using linear search.
void fifo_record(char *va, struct proc *curproc)
{
  int curpos = 0;
  struct memstab_page *curpg = curproc->memstab_head;

  while (curpg != 0)
  {
    for (curpos = 0; curpos < NUM_MEMSTAB_PAGE_ENTRIES; curpos++)
    {
      if (curpg->entries[curpos].vaddr == SLOT_USABLE)
      {
        curpg->entries[curpos].vaddr = va;
        curpg->entries[curpos].next = curproc->memqueue_head;
        if (curproc->memqueue_head == 0)
          curproc->memqueue_head = curproc->memqueue_tail = &(curpg->entries[curpos]);
        else
        {
          curproc->memqueue_head->prev = &(curpg->entries[curpos]);
          curproc->memqueue_head = &(curpg->entries[curpos]);
        }
        return;
      }
    }

    curpg = curpg->next;
  }

  panic("[ERROR] No free slot in memory.");
}

// Add a new page to memstab.
void record_page(char *va)
{
  struct proc *curproc = myproc();
  fifo_record(va, curproc);
  curproc->num_mem_entries++;
}

struct memstab_page_entry *fifo_write()
{
  struct memstab_page_entry *link, *last;
  struct proc *curproc = myproc();

  link = curproc->memqueue_head;
  if (link == 0 || link->next == 0)
    panic("Only 0 or 1 page in memory.");
  last = curproc->memqueue_tail;
  if (last == 0 || last->prev == 0)
    panic("[Error] last null!");
  curproc->memqueue_tail = last->prev;
  last->prev->next = 0;
  last->prev = 0;

  struct swapstab_page *curpage;
  int i = 0, pg = 0;

  curpage = curproc->swapstab_head;

  while (curpage != 0)
  {
    for (i = 0; i < NUM_SWAPSTAB_PAGE_ENTRIES; i++)
      if (curpage->entries[i].vaddr == SLOT_USABLE)
      {
        curpage->entries[i].vaddr = last->vaddr;
        if (swapwrite(curproc, (char *)PTE_ADDR(last->vaddr), (pg * SWAPSTAB_PAGE_OFFSET) + (i * PGSIZE), PGSIZE) == 0)
          return 0;
        goto SUCCESS;
      }
    curpage = curpage->next;
    pg++;
  }

  swapstab_growpage(curproc);
  curpage = curproc->swapstab_tail;

  for (i = 0; i < NUM_SWAPSTAB_PAGE_ENTRIES; i++)
    if (curpage->entries[i].vaddr == SLOT_USABLE)
    {
      curpage->entries[i].vaddr = last->vaddr;
      if (swapwrite(curproc, (char *)PTE_ADDR(last->vaddr), (pg * SWAPSTAB_PAGE_OFFSET) + (i * PGSIZE), PGSIZE) == 0)
        return 0;
      goto SUCCESS;
    }

  panic("[ERROR] SLOT OUT.");

  pte_t *pte;

SUCCESS:
  // Free the page pointed by last - it has been swapped out and can be reused.
  pte = walkpgdir(curproc->pgdir, (void *)last->vaddr, 0);
  if (!(*pte))
    panic("[ERROR] [fifo_write] PTE empty.");
  kfree((char *)(P2V_WO(PTE_ADDR(*pte))));
  *pte = PTE_W | PTE_U | PTE_PG;
  // Refresh page dir.
  lcr3(V2P(curproc->pgdir));

  // Return the freed slot.
  return last;
}

// Swap out a page from memstab to swapstab.
struct memstab_page_entry *write_page(char *va)
{
  if (SHOW_PAGE_SWAPOUT_INFO)
    cprintf("Swapping out a page.\n");
  return fifo_write();
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.

// We are using it to allocate memory from oldsz to newsz.
// Memory is not continuous now due to stack auto growth.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;
  struct proc* curproc = myproc();
  int stack_reserved = USERTOP - curproc->stack_size - PGSIZE;

  uint newpage_allocated = 1;
  struct memstab_page_entry *l;

  // Check args.
  if (curproc->stack_grow == 1)
  {
    // An empty page is reserved between stack and heap.
    if (oldsz == stack_reserved && oldsz < curproc->stack_size + PGSIZE)
      return 0;

    if (stack_reserved - PGSIZE < curproc->sz)
      return 0;
  }
  else if (newsz > stack_reserved)
  {
    return 0;
  }

  if (newsz > KERNBASE)
    return 0;
  if (newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for(; a < newsz; a += PGSIZE)
  {
    // Check if we have enough space to put the page in memory.
    if (curproc->num_mem_entries >= NUM_MEMSTAB_ENTRIES_CAPACITY)
    {
      // Swap out page at oldsz.
      if ((l = write_page((char *)a)) == 0)
        panic("[ERROR] Cannot write to swapfile.");

      l->vaddr = (char *)a;
      l->next = curproc->memqueue_head;
      if (curproc->memqueue_head == 0)
        curproc->memqueue_head = curproc->memqueue_tail = l;
      else
      {
        curproc->memqueue_head->prev = l;
        curproc->memqueue_head = l;
      }
      // No new page in memory will be used
      // (A page will be reused), mark that.
      newpage_allocated = 0;
    }

    mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }

    if (newpage_allocated)
      record_page((char *)a);

    memset(mem, 0, PGSIZE);
    if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
      cprintf("allocuvm out of memory (2)\n");
      deallocuvm(pgdir, newsz, oldsz);
      kfree(mem);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  pte_t *pte;
  uint a, pa;
  int i;
  struct proc* curproc = myproc();

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for (; a < oldsz; a += PGSIZE)
  {
    pte = walkpgdir(pgdir, (char *)a, 0);
    if (!pte)
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if ((*pte & PTE_P) != 0)
    {
      pa = PTE_ADDR(*pte);
      if (pa == 0)
        panic("kfree");

      // If the page is in memstab, clear it.
      if (curproc->pgdir == pgdir)
      {
        struct memstab_page *curpg = curproc->memstab_head;
        struct memstab_page_entry *slot = 0;

        while (curpg != 0)
        {
          for (i = 0; i < NUM_MEMSTAB_PAGE_ENTRIES; i++)
            if (curpg->entries[i].vaddr == (char *)(a))
            {
              slot = &(curpg->entries[i]);
              break;
            }

          if (slot == 0)
            curpg = curpg->next;
          else
            break;
        }

        panic("Should have a slot.");
        slot->vaddr = SLOT_USABLE;
        if (curproc->memqueue_head == slot)
        {
          if (slot->next != 0)
            slot->next->prev = 0;
          curproc->memqueue_head = slot->next;
        }
        else
        {
          struct memstab_page_entry *l = curproc->memqueue_head;
          while (l->next != slot)
            l = l->next;
          l->next->next->prev = l;
          l->next = slot->next;
        }
        slot->next = 0;
        slot->prev = 0;
        curproc->num_mem_entries--;
      }

      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
    // Maybe the page is not presented by is in swapfile.
    else if ((*pte & PTE_PG) && curproc->pgdir == pgdir)
    {
      struct swapstab_page* curpg;
      int i;

      curpg = curproc->swapstab_head;
      while(curpg!=0)
      {
        for(i = 0;i<NUM_SWAPSTAB_PAGE_ENTRIES;i++)
        {
          if(curpg->entries[i].vaddr == (char*)a)
          {
            curpg->entries[i].vaddr = SLOT_USABLE;
            return newsz;
          }
        }
        curpg = curpg->next;
      }

      panic("[ERROR] deallocuvm (entry not found (swap)).");
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
  uint i;

  if(pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, KERNBASE, 0);
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P){
      char * v = P2V(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void
clearpteu(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if(pte == 0)
    panic("clearpteu");
  *pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.

// This function has been modified.
// (Copy on write and stack auto growth.)
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i, flags;

  if((d = setupkvm()) == 0)
    return 0;

  // Copy code section, data section and heap section.
  for (i = PGSIZE; i < sz; i += PGSIZE)
  {
    if ((pte = walkpgdir(pgdir, (void *)i, 0)) == 0)
      panic("copyuvm: pte should exist");

    // Don't know why need to ignore the test.
    // Run "cat README | grep run" to see difference.
    if(!(*pte & PTE_P) && !(*pte & PTE_PG))
      continue;
    if (*pte & PTE_PG)
    {
      pte = walkpgdir(d, (void *)i, 1);
      *pte = PTE_U | PTE_W | PTE_PG;
      continue;
    }

    *pte &= ~PTE_W;
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if(mappages(d,(void*)i, PGSIZE, pa, flags) < 0)
      goto bad;
    incr_page_ref(pa);
  }

  // Copy stack section.
  // For simplicity we keep the stack shared.
  for(i = USERTOP - myproc()->stack_size;i<USERTOP;i+=PGSIZE)
  {
    if ((pte = walkpgdir(pgdir, (void *)i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if (!(*pte & PTE_P) && !(*pte & PTE_PG))
      continue;
    if (*pte & PTE_PG)
    {
      pte = walkpgdir(d, (void *)i, 1);
      *pte = PTE_U | PTE_W | PTE_PG;
      continue;
    }

    *pte &= ~PTE_W;
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if (mappages(d, (void *)i, PGSIZE, pa, flags) < 0)
      goto bad;
    incr_page_ref(pa);
  }

  lcr3(V2P(pgdir));
  return d;

bad:
  freevm(d);
  lcr3(V2P(pgdir));
  return 0;
}

//PAGEBREAK!
// Map user virtual address to kernel address.
char*
uva2ka(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if((*pte & PTE_P) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  return (char*)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
  char *buf, *pa0;
  uint n, va0;

  buf = (char*)p;
  while(len > 0){
    va0 = (uint)PGROUNDDOWN(va);
    pa0 = uva2ka(pgdir, (char*)va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (va - va0);
    if(n > len)
      n = len;
    memmove(pa0 + (va - va0), buf, n);
    len -= n;
    buf += n;
    va = va0 + PGSIZE;
  }
  return 0;
}

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.


//todo Refactor this messy code.
void pagefault(uint err_code)
{
  uint va = rcr2();
  struct proc* curproc = myproc();

  if(SHOW_PAGEFAULT_INFO)
    cprintf("pagefault at virt addr 0x%x, error code is %d, process name %s.\n", va, err_code, curproc->name);

  // If the page fault is caused by a non-present page,
  // should be due to lazy allocation or null pointer protection,
  // or stack needing growth, or due to be swapped out.
  // Otherwise, it should be due to protection violation (copy on write).
  // If the page fault is caused by kernel, it should be handled too.
  if (!(err_code & PGFLT_P))
  {
    // Used by swapping.
    pte_t* pte = &curproc->pgdir[PDX(va)];
    if(((*pte) & PTE_P) != 0)
    {
      // If the page is swapped out, swap it in.
      if(((uint*)PTE_ADDR(P2V(*pte)))[PTX(va)] & PTE_PG) 
      {
        swappage(PTE_ADDR(va));
        return;
      }
    }

    // If va is less than PGSIZE, this is a null pointer.
    if (va < PGSIZE)
    {
      cprintf("[ERROR] Dereferencing a null pointer (0x%x), \"%s\" will be killed.\n", va, curproc->name);
      curproc->killed = 1;
      return;
    }

    // If va is higher than sz and lower than stack top, should be stack growth.
    //? Is this always corrent?
    if (va >= curproc->sz + PGSIZE && va < USERTOP - curproc->stack_size)
    {
      if (SHOW_STACK_GROWTH_INFO)
        cprintf("[INFO ] Stack of \"%s\" is now growing.\n", curproc->name);
      curproc->stack_grow = 1;

      // An empty page is reserved between stack and heap.
      if (allocuvm(curproc->pgdir, USERTOP - curproc->stack_size - PGSIZE, USERTOP - curproc->stack_size) == 0)
      {
        cprintf("[ERROR] Stack growth failed, \"%s\" will be killed.\n", curproc->name);
        curproc->killed = 1;
      }
      curproc->stack_grow = 0;
      curproc->stack_size += PGSIZE;
      return;
    }

    // Otherwise, should be heap allocation.
    if (SHOW_LAZY_ALLOCATION_INFO)
      cprintf("Lazy allocation at virt addr 0x%x.\n", va);

    char *mem = kalloc();
    if (mem == 0)
    {
      cprintf("Lazy allocation failed: Memory out. Killing process.\n");
      curproc->killed = 1;
      return;
    }

    // va needs to be rounded down, or two pages will be mapped in mappages().
    va = PGROUNDDOWN(va);
    memset(mem, 0, PGSIZE);

    // The first process use this page can have write permissions,
    // but once forked, copyuvm will set it permission to readonly.
    if (mappages(curproc->pgdir, (char *)va, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0)
    {
      cprintf("Lazy allocation failed: Memory out (2). Killing process.\n");
      curproc->killed = 1;
      return;
    };
  
    return;
  }

  pte_t *pte;

  if (curproc == 0)
  {
    panic("Pagefault. No process.");
  }

  if ((va >= KERNBASE) || (pte = walkpgdir(curproc->pgdir, (void *)va, 0)) == 0 || !(*pte & PTE_P) || !(*pte & PTE_U))
  {
    if (SHOW_PAGEFAULT_IA_ERR)
      cprintf("Pagefault. Illegal address.\n");
    curproc->killed = 1;
    return;
  }

  if (*pte & PTE_W)
  {
    panic("Pagefault. Already writeable.");
  }

  uint pa = PTE_ADDR(*pte);
  ushort ref = get_page_ref(pa);
  char *mem;

  if (ref == 1)
    *pte |= PTE_W;
  else if (ref > 1)
  {
    if ((mem = kalloc()) == 0)
    {
      cprintf("Pagefault. Out of memory.");
      curproc->killed = 1;
      return;
    }

    memmove(mem, P2V(pa), PGSIZE);
    *pte = V2P(mem) | PTE_P | PTE_U | PTE_W;
    decr_page_ref(pa);
  }
  else
    panic("Pagefault. Reference count error.");
}

void fifo_swap(uint addr)
{
  int i, j;
  char buf[SWAP_BUF_SIZE];
  pte_t *pte_in, *pte_out;
  struct proc *curproc = myproc();

  // Find the last record in memstab.
  struct memstab_page_entry *link = curproc->memqueue_head;
  struct memstab_page_entry *last;
  if (link == 0 || link->next == 0)
    panic("[ERROR] Only 0 or 1 pages in memory.");
  last = curproc->memqueue_tail;
  if (last == 0 || last->prev == 0)
    panic("[ERROR] last null!");
  curproc->memqueue_tail = last->prev;
  last->prev->next = 0;
  last->prev = 0;

  // Locate the PTE of the page to be swapped out.
  pte_in = walkpgdir(curproc->pgdir, (void *)last->vaddr, 0);
  if (!*pte_in)
    panic("[ERROR] A record is in memstab but not in pgdir.");

  struct swapstab_page *curpg = 0;
  struct swapstab_page_entry *ent = 0;
  uint offset = 0;

  curpg = curproc->swapstab_head;

  // Find the record of the page to be swapped in in swap_pages.
  while (curpg != 0)
  {
    for (i = 0; i < NUM_SWAPSTAB_PAGE_ENTRIES; i++)
      if (curpg->entries[i].vaddr == (char *)PTE_ADDR(addr))
      {
        ent = &(curpg->entries[i]);
        offset += i * PGSIZE;
        break;
      }

    if (ent != 0)
      break;
    else
    {
      curpg = curpg->next;
      offset += SWAPSTAB_PAGE_OFFSET;
    }
  }

  if (ent == 0)
    panic("[ERROR] Should find a record in swapfile!");

  // Perform swap.
  ent->vaddr = last->vaddr;

  pte_out = walkpgdir(curproc->pgdir, (void *)addr, 0);
  if (!*pte_out)
    panic("[ERROR] A record should be in pgdir!");
  *pte_out = PTE_ADDR(*pte_in) | PTE_U | PTE_W | PTE_P;

  // Real swap - read from swapfile and write to swap file.
  for (j = 0; j < 4; j++)
  {
    uint loc = offset + (SWAP_BUF_SIZE * j);
    int off = SWAP_BUF_SIZE * j;
    memset(buf, 0, SWAP_BUF_SIZE);
    swapread(curproc, buf, loc, SWAP_BUF_SIZE);
    swapwrite(curproc, (char *)(P2V_WO(PTE_ADDR(*pte_in)) + off), loc, SWAP_BUF_SIZE);
    memmove((void *)(PTE_ADDR(addr) + off), (void *)buf, SWAP_BUF_SIZE);
  }

  *pte_in = PTE_U | PTE_W | PTE_PG;
  last->next = curproc->memqueue_head;
  curproc->memqueue_head->prev = last;
  curproc->memqueue_head = last;
  last->vaddr = (char *)PTE_ADDR(addr);
}

void swappage(uint addr)
{
  if(SHOW_SWAPPAGE_INFO)
    cprintf("[ INFO ] Swapping page for 0x%x.\n", addr);
    
  struct proc*curproc = myproc();

  //? Why should we do this?
  if (kstrcmp(curproc->name, "init") == 0 || kstrcmp(curproc->name, "sh") == 0)
  {
    curproc->num_mem_entries++;
    return;
  }

  fifo_swap(addr);

  // Refresh page dir.
  lcr3(V2P(curproc->pgdir));
}

// init shared memory
void initshm(void)
{
  int i;
  for (i = 0; i < MAX_SHMNODE_NUM; i++)
  {
    shmlist[i].sig = 0;
    shmlist[i].count = 0;
    shmlist[i].pagenum = 0;
    shmlist[i].addr = 0;
  }
}

// increase some sig counts in shmlist
// add permission to now process
// if sig does not exist, then create shared physical pages that can cover data of size "bytes".
// return 1 when succeed create, return 0 when sig already exists, return -1 when fail.
int createshm(uint sig, uint bytes, int type)
{
  if (sig == 0)
  {
    cprintf("cannot use sig 0 for shared memory, createshm failed!\n");
    return -1;
  }
  if (bytes > 1024 * PGSIZE)
  {
    cprintf("%d bytes is too big, cannot get that big shared area, createshm failed!\n", bytes);
    return -1;
  }
  if(type !=0 && type != 1)
  {
    cprintf("error shmtype: %d\n", type);
    return -1;
  }
  acquire(&kshmlock);
  int firstzero = -1; //first position to insert a new sig
  int i;
  for (i = 0; i < MAX_SHMNODE_NUM; i++)
  {
    if (firstzero < 0 && shmlist[i].sig == 0)
    {
      firstzero = i;
    }
    if (shmlist[i].sig == sig)
    {
      break;
    }
  }
  if(i != MAX_SHMNODE_NUM && shmlist[i].type != type)// not match
  {
    cprintf("shared block type did not match with the former type in this block!\n");
    release(&kshmlock);
    return -1;
  }
  struct proc *pr = myproc();
  int j;
  for (j = 0; j < MAX_SIG_PER_PROC; j++)
  {
    if (pr->sig_permit[j] == sig)
    {
      cprintf("this process already had the permission to shared area %d, cannot get permission again, createshm failed!\n", sig);
      release(&kshmlock);
      return -1;
    }
  }
  for (j = 0; j < MAX_SIG_PER_PROC; j++)
  {
    if (pr->sig_permit[j] == 0)
    {
      break;
    }
  }
  if (j == MAX_SIG_PER_PROC) //overflow, already have 5 sigs in this proc
  {
    cprintf("this process already have 5 sigs in this process, cannot add more shared memory area to access, createshm failed\n");
    release(&kshmlock);
    return -1;
  }
  pr->sig_permit[j] = sig;
  if (i != MAX_SHMNODE_NUM) //this sig already exists
  {
    shmlist[i].count++;
    if (bytes > shmlist[i].pagenum * PGSIZE) //shared area "sig" need extend
    {
      int target = PGROUNDUP(bytes) / PGSIZE;
      for (int j = shmlist[i].pagenum; j < target; j++)
      {
        if ((shmlist[i].addr->addrs[j] = kalloc()) == 0)
        {
          cprintf("kalloc failed\n");
          for (int j0 = shmlist[i].pagenum; j0 < j; j0++)
            kfree(shmlist[i].addr->addrs[j0]);
          release(&kshmlock);
          return -1;
        }
      }
    }
    release(&kshmlock);
    return 0;
  }
  else //create a new sig in pos
  {
    int pos = firstzero == -1 ? i : firstzero;
    shmlist[pos].sig = sig;
    shmlist[pos].count = 1;
    shmlist[pos].pagenum = PGROUNDUP(bytes) / PGSIZE;
    shmlist[pos].type = type;
    if ((shmlist[pos].addr = (struct shmindex *)kalloc()) == 0)
    {
      cprintf("kalloc failed\n");
      release(&kshmlock);
      return -1;
    }
    int i0;
    for (i0 = 0; i0 < shmlist[pos].pagenum; i0++)
    {
      if ((shmlist[pos].addr->addrs[i0] = kalloc()) == 0)
      {
        cprintf("kalloc failed\n");
        for (j = 0; j < i0; j++)
          kfree(shmlist[pos].addr->addrs[j]);
        release(&kshmlock);
        return -1;
      }
    }
    for (i = shmlist[pos].pagenum; i < 1024; i++)
      shmlist[pos].addr->addrs[i] = (char *)0xffffffff;
    release(&kshmlock);
    return 0;
  }
}

// decrease some sig counts in shmlist
// remove permission to now proc
// if count is zero, then delete shared physical pages pointed by sig.
// return 1 when succeed, return 0 when delelte a sig, return -1 when fail.
int deleteshm(uint sig)
{
  if (sig == 0)
  {
    cprintf("cannot use sig 0 for shared memory, deleteshm failed!\n");
    return -1;
  }
  acquire(&kshmlock);
  int i;
  for (i = 0; i < MAX_SHMNODE_NUM; i++)
  {
    if (shmlist[i].sig == sig)
    {
      break;
    }
  }
  if (i == MAX_SHMNODE_NUM) //shared memory pages do not exist
  {
    cprintf("sig does not existed, deleteshm failed!\n");
    release(&kshmlock);
    return -1;
  }
  struct proc *pr = myproc();
  int j;
  for (j = 0; j < MAX_SIG_PER_PROC; j++)
  {
    if (pr->sig_permit[j] == sig)
    {
      pr->sig_permit[j] = 0;
      break;
    }
  }
  if (j == MAX_SIG_PER_PROC) //not found
  {
    cprintf("this process does not have permission to access sig, deleteshm failed!\n");
    release(&kshmlock);
    return -1;
  }
  shmlist[i].count--;
  if (shmlist[i].count == 0) //delete sig node,free addr
  {
    shmlist[i].sig = 0;
    int k;
    for (k = 0; k < shmlist[i].pagenum; k++)
    {
      kfree(shmlist[i].addr->addrs[k]);
    }
    kfree((char *)shmlist[i].addr);
  }
  release(&kshmlock);
  return 1;
}

// write data from wstr to shared pages with offset "offset"
// data length is num
// return 0 when succeed and return -1 when failed
int writeshm(uint sig, char *wstr, uint num, uint offset)
{
  if (sig == 0)
  {
    cprintf("cannot use sig 0 for shared memory, writeshm failed!\n");
    return -1;
  }
  int i;
  for (i = 0; i < MAX_SHMNODE_NUM; i++)
  {
    if (shmlist[i].sig == sig)
      break;
  }
  if (i == MAX_SHMNODE_NUM) //does not exist
  {
    cprintf("shared area %d has not been created, writeshm failed!\n", sig);
    return -1;
  }
  int serial_mark = 0;
  if (shmlist[i].type == SERIAL)
  {
    serial_mark = 1;
  }
  if(serial_mark)
  {
    acquire(&kshmlock);
  }
  struct proc *pr = myproc();
  int j;
  for (j = 0; j < MAX_SIG_PER_PROC; j++)
  {
    if (pr->sig_permit[j] == sig)
    {
      break;
    }
  }
  if (j == MAX_SIG_PER_PROC) //not found
  {
    cprintf("this process does not have permission to access shared area %d, writeshm failed!\n", sig);
    if(serial_mark)
    {
      release(&kshmlock);
    }
    return -1;
  }

  if (num > PGSIZE * shmlist[i].pagenum - offset) //overflow
  {
    cprintf("write area overflow, not enough space to write in %d bytes, writeshm failed!\n", num);
    if(serial_mark)
    {
      release(&kshmlock);
    }
    return -1;
  }

  int pageindex = offset / PGSIZE; //start page
  int offs = offset % PGSIZE; //offset in one page
  for (int j = num; j > 0; j--)
  {
    shmlist[i].addr->addrs[pageindex][offs++] = wstr[num - j];
    if (offs == PGSIZE) //change page
    {
      pageindex++;
      offs = 0;
    }
  }
  if(serial_mark)
  {
    release(&kshmlock);
  }
  return 0;
}

// read data to rstr from shared pages with offset "offset"
// return 0 when succeed and return -1 when failed
int readshm(uint sig, char *rstr, uint num, uint offset)
{
  if (sig == 0)
  {
    cprintf("cannot use sig 0 for shared memory, readshm failed!\n");
    return -1;
  }
  int i;
  for (i = 0; i < MAX_SHMNODE_NUM; i++)
  {
    if (shmlist[i].sig == sig)
      break;
  }
  if (i == MAX_SHMNODE_NUM) //does not exist
  {
    cprintf("shared area %d has not been created, readshm failed!\n", sig);
    return -1;
  }
  int serial_mark = 0;
  if(shmlist[i].type == SERIAL)
  {
    serial_mark = 1;
    acquire(&kshmlock);
  }
  struct proc *pr = myproc();
  int j;
  for (j = 0; j < MAX_SIG_PER_PROC; j++)
  {
    if (pr->sig_permit[j] == sig)
    {
      break;
    }
  }
  if (j == MAX_SIG_PER_PROC) //not found
  {
    cprintf("this process does not have permission to access shared area %d, readshm failed!\n", sig);
    if(serial_mark)
      release(&kshmlock);
    return -1;
  }

  if (num > PGSIZE * shmlist[i].pagenum - offset) //overflow, read all saved data 
  {
    num = PGSIZE * shmlist[i].pagenum - offset;
  }

  int pageindex = offset / PGSIZE; //start page
  int offs = offset % PGSIZE;      //offset in one page
  for (int j = num; j > 0; j--)
  {
    rstr[num - j] = shmlist[i].addr->addrs[pageindex][offs++];
    if (offs == PGSIZE) //change page
    {
      pageindex++;
      offs = 0;
    }
  }
  if(serial_mark)
      release(&kshmlock);
  return 0;
}

int getsharedpages(void)
{
  int pgnum = 0;
  int i = 0;
  for(;i<MAX_SHMNODE_NUM;i++)
  {
    pgnum += shmlist[i].pagenum;
  }
  return pgnum;
}
