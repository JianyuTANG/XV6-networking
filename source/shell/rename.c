/*
*文件名称：rename.c
*创建者：陈倩
*创建日期：2018/06/07
*文件描述：文件重命名
*历史记录：整合自三字班方案二
*/
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf(2, "Usage: rename filename newfilename\n");
    exit();
  }
  if(link(argv[1], argv[2]) < 0){
    printf(2, "ren: %s failed to rename\n", argv[1]);
    exit();
  }
  if(unlink(argv[1]) < 0){
    printf(2, "ren: %s failed to unlink the old name\n", argv[1]);
    exit();
  }
  chdir("/");
  exit();
}