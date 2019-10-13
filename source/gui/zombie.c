// Create a zombie process that
// must be reparented at exit.

#include "types.h"
#include "stat.h"
#include "user.h"

void f()
{
  if (fork() > 0)
  {
    printf(1, "child\n");
  }
  else
    printf(1, "parent\n");
  printf(1, "f\n");
}

int
main(void)
{
  f();
  printf(1, "main\n");
  exit();
}
