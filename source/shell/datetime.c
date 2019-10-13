/*
*文件名称：datetime.c
*创建者：程嘉梁
*创建日期：2018/04/16
*文件描述：实时时间
*历史记录：整合自三字班方案二
*/
#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"


void datetime(void* date)
{
  cmostime(date);
}
