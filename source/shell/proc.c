#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void 
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}


// Initialize the swap table. It will set all relating fields to zero in process table.
void swaptableinit(void)
{
  int i;
  struct proc *thisproc;
  acquire(&ptable.lock);
  for (i = 0; i < NPROC; i++)
  {
    thisproc = &ptable.proc[i];
    thisproc->num_mem_entries = 0;
    thisproc->memstab_head = 0;
    thisproc->memstab_tail = 0;
    thisproc->swapstab_head = 0;
    thisproc->swapstab_tail= 0;
    thisproc->memqueue_head = 0;
    thisproc->memqueue_tail = 0;
    thisproc->num_swapstab_pages = 0;

    int j;
    for (j = 0; j < MAX_SWAPFILES; j++)
      thisproc->swapfile[j] = 0;
  }
  release(&ptable.lock);
}

// Clear one memory swap table page. If clear link is set to true,
// it will also clear links (pointer to prev page and next page).
void memstab_page_clear(struct memstab_page *page, uint clear_link)
{
  int i;
  for (i = 0; i < NUM_MEMSTAB_PAGE_ENTRIES; i++)
  {
    page->entries[i].prev = 0;
    page->entries[i].next = 0;
    page->entries[i].vaddr = SLOT_USABLE;
  }
  if (clear_link)
  {
    page->next = 0;
    page->prev = 0;
  }
}

// Allocate one memory swap table page. Links are set to NULL initially.
struct memstab_page *memstab_page_alloc(void)
{
  struct memstab_page *mstabpg;
  if ((mstabpg = (struct memstab_page *)kalloc()) == 0)
    return 0;
  memstab_page_clear(mstabpg, 1);
  return mstabpg;
}

// Clear a process's memory swap table. But the memory is not released.
// This function is designed for reuse the allocated memory.
void memstab_clear(struct proc *pr)
{
  struct memstab_page *p = pr->memstab_head;
  while (p != 0)
  {
    memstab_page_clear(p, 0);
    p = p->next;
  }
  pr->num_mem_entries = 0;
  pr->memqueue_head = 0;
  pr->memqueue_tail = 0;
}

// Allocate a full memory swap table, return its head address.
// The swaptable is linked.
struct memstab_page * memstab_alloc(void)
{
  int i;
  struct memstab_page *slow, *fast, *head;
  if ((slow = memstab_page_alloc()) == 0)
    return 0;
  head = slow;
  for (i = 0; i < NUM_MEMSTAB_PAGES - 1; i++)
  {
    if ((fast = memstab_page_alloc()) == 0)
      return 0;
    slow->next = fast;
    fast->prev = slow;
    slow = fast;
  }

  return head;
}

// Clear one swapped swap table page. If clear_link is set, it will also clear pointers to prev and next page.
void swapstab_page_clear(struct swapstab_page *page, uint clear_link)
{
  int i;
  for (i = 0; i < NUM_SWAPSTAB_PAGE_ENTRIES; i++)
    page->entries[i].vaddr = SLOT_USABLE;
  if (clear_link)
  {
    page->next = 0;
    page->prev = 0;
  }
}

// Allocate one swapped swap table page. Links are NULL initially.
struct swapstab_page* swapstab_page_alloc(void)
{
  struct swapstab_page *sstabpg;
  if ((sstabpg = (struct swapstab_page *)kalloc()) == 0)
    return 0;
  swapstab_page_clear(sstabpg, 1);
  return sstabpg;
}

// Clear the swapped swap table for a process. 
// Preserve space and link relation.
void swapstab_clear(struct proc *pr)
{
  struct swapstab_page *p;

  p = pr->swapstab_head;
  while (p != 0)
  {
    swapstab_page_clear(p, 0);
    p = p->next;
  }
}

int swapstab_growpage(struct proc *pr)
{
  struct swapstab_page **head, **tail;
  head = &(pr->swapstab_head);
  tail = &(pr->swapstab_tail);
  
  // Start growing.
  if (*head == 0)
  {
    // This process has no swapped swap page.
    if ((*head = swapstab_page_alloc()) == 0)
      return -1;
    *tail = *head;
  }
  else
  {
    // This process has some swapped swap page.
    struct swapstab_page *temp = *tail;
    if ((*tail = swapstab_page_alloc()) == 0)
      return -1;
    temp->next = *tail;
    (*tail)->prev = temp;
  }

  return 0;
}

// Copy swap table (mem, swapped) from srcproc to dstproc.
// Don't preserve relative location in memory swap table.
// Returns 0 on success, otherwise -1.
int copy_stab(struct proc *dstproc, struct proc *srcproc)
{
  // Copy memory swap table.
  memstab_clear(dstproc);
  dstproc->num_mem_entries = srcproc->num_mem_entries;
  dstproc->memqueue_head = 0;
  dstproc->memqueue_tail = 0;

  struct memstab_page *curpg = dstproc->memstab_head;
  int curpos = 0;
  struct memstab_page_entry *cursrcent = srcproc->memqueue_head;
  struct memstab_page_entry *olddstent = 0;

  if (cursrcent != 0)
    dstproc->memqueue_head = &(curpg->entries[curpos]);
  while (cursrcent != 0)
  {
    if (olddstent != 0)
      olddstent->next = &(curpg->entries[curpos]);
    curpg->entries[curpos].prev = olddstent;
    olddstent = &(curpg->entries[curpos]);
    curpg->entries[curpos].vaddr = cursrcent->vaddr;

    cursrcent = cursrcent->next;
    curpos++;

    if (curpos == NUM_MEMSTAB_PAGE_ENTRIES)
    {
      curpg = curpg->next;
      curpos = 0;
    }
  }
  dstproc->memqueue_tail = olddstent;

  // Copy swapped swap table.
  int i;
  struct swapstab_page *srccurpg, *dstcurpg;
  while (srcproc->num_swapstab_pages > dstproc->num_swapstab_pages)
  {
    if (swapstab_growpage(dstproc) == 0)
      return -1;
  }

  srccurpg = srcproc->swapstab_head;
  dstcurpg = dstproc->swapstab_head;
  while (srccurpg != 0)
  {
    for (i = 0; i < NUM_SWAPSTAB_PAGE_ENTRIES; i++)
      dstcurpg->entries[i] = srccurpg->entries[i];
    dstcurpg = dstcurpg->next;
    srccurpg = srccurpg->next;
  }

  return 0;
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  // Set up mem swap table if not exist.
  // Otherwise clear it.
  if (p->memstab_head == 0)
  {
    if ((p->memstab_head = memstab_alloc()) == 0)
      return 0;
  }
  else
    memstab_clear(p);

  // Set up data for page swapping.
  p->num_mem_entries = 0;
  p->memqueue_head = 0;
  p->memqueue_tail = 0;

  //Initialize sigs
  for(int k = 0;k < MAX_SIG_PER_PROC;k++)
  {
    p->sig_permit[k] = 0;
  }
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();

  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  p->cwdname[0] = '/';
  p->cwdname[1] = '\0';

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;

  if (sz + n > USERTOP - curproc->stack_size - PGSIZE)
    return -1;

  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;  
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  np->stack_size = curproc->stack_size;
  *np->tf = *curproc->tf;

  // Copy data for swapping.
  np->num_mem_entries = curproc->num_mem_entries;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);
  safestrcpy(np->cwdname, curproc->cwdname, sizeof(curproc->cwdname));
  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  swapalloc(np);
  char buf[PGSIZE / 2] = "";
  int offset = 0;
  int nread = 0;

  if (kstrcmp(curproc->name, "init") != 0 && kstrcmp(curproc->name, "sh") != 0)
  {
    // Copy swap file.
    offset = 0;
    nread = 0;
    while ((nread = swapread(curproc, buf, offset, PGSIZE / 2)) != 0)
    {
      if (swapwrite(np, buf, offset, nread) == -1)
        panic("[ERROR] Copying swapfile in fork().");
      offset += nread;
    }
  }

  // Copy data for swapping.
  if (copy_stab(np, curproc) == -1)
    return -1;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  for(int ptr = 0;ptr < MAX_SIG_PER_PROC;ptr++)
  {
    np->sig_permit[ptr] = 0;
  }
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  // Remove all sigs
  for(int i = 0;i < MAX_SIG_PER_PROC;i++)
  {
    if(curproc->sig_permit[i] != 0)
      deleteshm(curproc->sig_permit[i]);
  }

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  // Remove swap file.
  if (swapdealloc(curproc) != 0)
    panic("[ERROR] Remove swap file error.");

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;
  memset(curproc->cwdname,0,1000*4);

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
    
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

//Make parentpid become pid's parent
void 
reparent(int pid,int parentpid)
{
  struct proc *p;
  struct proc *parent = 0;

  acquire(&ptable.lock);

  for (p = ptable.proc; p < &ptable.proc[NPROC];p++){
    if(p->pid == parentpid)
      parent = p;
  }
  if(parent == 0){
    release(&ptable.lock);
    return;
  }

  for (p = ptable.proc; p < &ptable.proc[NPROC];p++){
    if(p->pid == pid){
      p->parent = parent;
      release(&ptable.lock);
      return;
    }
  }
  release(&ptable.lock);
  return;
}

//Get pid's current running state
int
getstate(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      release(&ptable.lock);
      return p->state;
    }
  }
  release(&ptable.lock);
  return -1;
}

//Suspend current foreground process
int
suspend(void)
{
  struct proc *p;
  cprintf("\nCtrl+C detected\n");
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if (p != initproc && p->pid != 2 && (p->state == RUNNING || p->state == SLEEPING)) {
      p->killed = 1;
      return kill(p->pid);
    }
  }
  return -1;
}

int
getprocinfo(int *pid, char (*name)[16], int *state, uint *sz)
{
  struct proc p;
  int i, j;

  //将进程信息存入pid、name、state和sz中
  for(i = 0; i < NPROC; i++)
  {
    p = ptable.proc[i];
    pid[i] = p.pid;
    for(j = 0; j < 16; j++)
      name[i][j] = p.name[j];
    state[i] = p.state;
    sz[i] = p.sz;
  }
  return 0;
}


void
showproc(void)
{
  cprintf("Process list:\n");
  cprintf("Name\tHeap\tStack\t\n");
  struct proc* p;
  for(p=ptable.proc;p<&ptable.proc[NPROC];p++)
  {
    if(p->state==RUNNING || p->state==RUNNABLE || p->state==SLEEPING)
    {
      cprintf("%s\t%d\t%d\n",p->name,p->sz,p->stack_size);
    }
  }
}
