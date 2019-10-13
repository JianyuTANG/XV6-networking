/*
*文件名称：shutdown.c
*创建者：程嘉梁
*创建日期：2018/05/05
*文件描述：实现Xv6关机操作（shutdown命令）
*资料来源:http://pdos.csail.mit.edu/6.828/2012/homework/xv6-syscall.html
*/

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(2,"Shutting down...\n");
  shutdown();
  return 0;
}

