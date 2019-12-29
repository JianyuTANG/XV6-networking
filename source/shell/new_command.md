# Instructions for adding new command

Assumed the name of the new command to be created is `cmds`.

## user.h
add the declaration of your new command like

```C
int sys_cmds(void);
```

## sysfile.c
add the real code of your command here like

```C
int sys_cmds(void)
{
    // code

    return 0;
}
```


## cmds.c
create a new file named `cmds.c` in the root, a quick example can be found in `ifconfig.c`.

## syscall.h
add

```C
#define SYS_cmds  55
```

## syscall.c
add 

```C
extern int sys_cmds(void);

static int (*syscalls[])(void) = {
// ...
[SYS_cmds] sys_cmds,
// ...
}
```

## usys.S
add 

```
SYSCALL(cmds)
```

## Makefile
add

```makefile
UPROGS = \
# ...
    _cmds
```
