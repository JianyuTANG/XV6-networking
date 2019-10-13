OBJCOPY = objcopy
OBJDUMP = objdump
RANLIB = ranlib
QEMU = qemu-system-i386
INCLDIR = ../include

CFLAGS += -nostdinc -fno-builtin -fno-stack-protector -std=gnu99 -Wall -Wextra -m32 -I$(INCLDIR)

# Release or Debug?
#CFLAGS += -O2
CFLAGS +=  -fvar-tracking-assignments -g -ggdb

ASFLAGS += -m32 -Wa,-divide -nostdinc -I$(INCLDIR)
LDFLAGS += -m elf_i386

# Debug Settings
GDBPORT = 26000

# Emulation Settings
CPUS = 2
RAM = 512

ULIB = string.o math.o stdlib.o stdio.o scanf.o ctype.o setjmp.o time.o

all: libc.a

install:

libc.a: $(ULIB)
	$(AR) vr $@ $^
	$(RANLIB) $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

%.o: %.S
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f *.o libc.a

.PHONY: all install clean
