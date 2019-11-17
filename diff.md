**增加了**

- util.h/util.c 实现了atoi函数和strcmp函数
- pci.c/pci.h/pciregister.h 应该是pci相关的内容
- arp* 实现arp协议
- e1000.h/e1000.c 网卡驱动
- nic.h/nic.c 较短 目的暂时不清楚



**修改了**

- asm.h 增加了几个常数值
- defs.h 增加了与arp.c相关内容
- forktest.c 少了一个const?
- fs.c
- main.c 主函数
- Makefile
- mmu.h 多了一堆常数
- picirq.c 
- sleeplock.c/spinlock.c 可能是xv6的更新
- syscall.h/syscall.c 加入了一些声明
- sysfile.c
- trap.c
- traps.h
- user.h
- usys.S
- vm.c
- x86.h