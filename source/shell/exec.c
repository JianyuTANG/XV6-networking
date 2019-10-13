#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

extern char _binary_lua_start[], _binary_lua_end[];

void kmemcpy(char *src, char *dest, uint n) {
  while (n != 0) {
    *(dest++) = *(src++);
    n--;
  }
}

int exec_strcmp(char *src, char *dest) {
  while (*src == *dest) {
    if (*src == 0) {
      return 0;
    }
    src++;
    dest++;
  }
  return (*(unsigned char *) src - *(unsigned char *) dest);
}

int readm(char *src, char *dest, uint off, uint n, const char *end) {
  if (src + off > end) {
    return -1;
  }
  if (off + n < off) {
    return -1;
  }
  if (src + off + n > end) {
    n = end - src - off;
  }
  kmemcpy(src + off, dest, n);
  return n;
}

int load_from_kernel(char code_start[], char code_end[], pde_t **ppgdir, uint *psz, struct proghdr *pph,
                     struct elfhdr *pelf) {
  if (!code_start) {
    cprintf("exec: fail\n");
    return -1;
  }

  *ppgdir = 0;

  if (readm(code_start, (char *) pelf, 0, sizeof(*pelf), code_end) != sizeof(*pelf)) {
    return -1;
  }
  if (pelf->magic != ELF_MAGIC) {
    return -1;
  }

  if ((*ppgdir = setupkvm()) == 0) {
    return -1;
  }

  *psz = 0;
  for (int i = 0, off = pelf->phoff; i < pelf->phnum; i++, off += sizeof(*pph)) {
    if (readm(code_start, (char *) pph, off, sizeof(*pph), code_end) != sizeof(*pph)) {
      return -1;
    }
    if (pph->type != ELF_PROG_LOAD) {
      continue;
    }
    if (pph->memsz < pph->filesz) {
      return -1;
    }
    if (pph->vaddr + pph->memsz < pph->vaddr) {
      return -1;
    }
    if ((*psz = (uint) allocuvm(*ppgdir, *psz, pph->vaddr + pph->memsz)) == 0) {
      return -1;
    }
    if (pph->vaddr % PGSIZE != 0) {
      return -1;
    }
    if (loaduvm_from_kernel(*ppgdir, (char *) pph->vaddr, code_start, code_end,
                            pph->off, pph->filesz) < 0) {
      return -1;
    }
  }

  return 0;
}

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  if (exec_strcmp(path, "lua") == 0) {

    ip = 0;

    if (load_from_kernel(_binary_lua_start, _binary_lua_end, &pgdir, &sz, &ph, &elf) < 0) {
      goto bad;
    }

  } else {

    begin_op();

    if((ip = namei(path)) == 0){
      end_op();
      cprintf("exec: fail\n");
      return -1;
    }
    ilock(ip);
    pgdir = 0;

    // Check ELF header
    if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
      goto bad;
    if(elf.magic != ELF_MAGIC)
      goto bad;

    if((pgdir = setupkvm()) == 0)
      goto bad;

    // Save data for swapping, restore them later.
    // Normalily we can just clear them, but if exec failed,
    // we need to be able to restore.

    memstab_clear(curproc);
    swapstab_clear(curproc);

    //todo Need a mechanism to save the swap table and restore it if exec fails.

    // Load program into memory.
    sz = PGSIZE;
    for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
      if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
        goto bad;
      if(ph.type != ELF_PROG_LOAD)
        continue;
      if(ph.memsz < ph.filesz)
        goto bad;
      if(ph.vaddr + ph.memsz < ph.vaddr)
        goto bad;
      if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
        goto bad;
      if(ph.vaddr % PGSIZE != 0)
        goto bad;
      if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
        goto bad;
    }
    iunlockput(ip);
    end_op();
    ip = 0;

  }

  // Set sz to heap bottom.
  sz = PGROUNDUP(sz);

  // Setup the top page for stack.
  curproc->stack_grow = 1;
  if (allocuvm(pgdir, USERTOP - PGSIZE, USERTOP) == 0)
    goto bad;
  curproc->stack_grow = 0;

  sp = USERTOP;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->stack_size = PGSIZE;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;

  // Refresh swapfile.
  swapdealloc(curproc);
  swapalloc(curproc);

  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }

  // Save data for swapping, restore them later.
  // Normalily we can just clear them, but if exec failed,
  // we need to be able to restore.
  //todo Need a mechanism to save the swap table and restore it if exec fails.

  return -1;
}
