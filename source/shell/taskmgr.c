#include "types.h"
#include "stat.h"
#include "user.h"

#define NPROC 64  // maximum number of processes
#define KEY_UP 0xE2
#define KEY_DN 0xE3
#define KEY_LF 0xE4
#define KEY_RT 0xE5

char procstate[6][9] = {"UNUSED", "EMBRYO", "SLEEPING", "RUNNABLE", "RUNNING", "ZOMBIE"};
char digits[] = "0123456789";
char blankprocinfo[] = "0                   0                   0                   0";

char scrbuff[96][80];
int curline = 4;  //表示当前高亮显示的行序号，范围4-19
int curpage = 0;  //表示当前页码，范围0-3
int procID[NPROC];
char procName[NPROC][16];
int procState[NPROC];
uint procSize[NPROC];

void runTaskMgr(void);
int getCmd(char *buf);
void procCmd(char *cmd);
void initTaskMgrInfo(void);
void clearTaskMgrInfo(void);
void updTaskMgrInfo(void);

void
runTaskMgr(void)
{
  char buf[2];

  initTaskMgrInfo();
  updTaskMgrInfo();
  updscrcont(scrbuff[curpage*24], curline);
  while(getCmd(buf) >= 0){
    procCmd(buf);
    updTaskMgrInfo();
    updscrcont(scrbuff[curpage*24], curline);
  }
}

int
getCmd(char *buf)
{
  memset(buf, 0, sizeof(char) * 2);
  gets(buf, 2);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

void
procCmd(char *cmd)
{
  switch((*cmd) & 0xff){
  case KEY_UP:
    if(curline > 4)
      curline--;
    break;
  case KEY_DN:
    if(curline < 19)
      curline++;
    break;
  case KEY_LF:
    if(curpage > 0)
      curpage--;
    break;
  case KEY_RT:
    if(curpage < 3)
      curpage++;
    break;
  case 'k':
    if(strcmp("taskmgr", procName[curpage*16+curline-4]) == 0){
      closetaskmgr();
      exit();
      break;
    }
    else{
      kill(procID[curpage*16+curline-4]);
      break;
    }
  case 'q':
    closetaskmgr();
    exit();
    break;
  case 'f':
    break;
  default:
    break;
  }
}

void
initTaskMgrInfo(void)
{
  char title1[] = "Task Manager";
  char title2[] = "pid                 name                state               memory(bytes)";
  char footNote[] = "page: ";
  int page, row, col;
  char *s;

  for(page = 0; page < 4; page++)
    for(row = 0; row < 24; row++)
      for(col = 0; col < 80; col++)
        scrbuff[24*page+row][col] = '\0';

  for(page = 0; page < 4; page++){
    for(col = 34, s = title1; *s != '\0'; s++)
      scrbuff[24*page][col++] = *s;
    for(col = 0, s = title2; *s != '\0'; s++)
      scrbuff[24*page+2][col++] = *s;
    for(col = 70, s = footNote; *s != '\0'; s++)
      scrbuff[24*page+23][col++] = *s;
    scrbuff[24*page+23][col++] = page+'1';
    scrbuff[24*page+23][col++] = '/';
    scrbuff[24*page+23][col++] = '4';
  }
}

void
clearTaskMgrInfo(void)
{
  int page, row, col;

  for(page = 0; page < 4; page++)
    for(row = 4; row < 20; row++)
      for(col = 0; col < 80; col++)
        scrbuff[24*page+row][col] = '\0';
}

void
updTaskMgrInfo(void)
{
  int page, row, col, x, i;
  char *s;
  int localbuff[20];
  
  getprocinfo(procID, procName, procState, procSize);
  for(page = 0; page < 4; page++)
    for(row = 4; row < 20; row++)
      if(procID[16*page+row-4] <= 0)
        for(col = 0, s = blankprocinfo; *s != '\0'; s++)
          scrbuff[24*page+row][col++] = *s;
      else{
        for(i = -1, x = procID[16*page+row-4]; x > 0; x/=10)
          localbuff[++i] = x%10;
        for(col = 0; i >= 0; i--)
          scrbuff[24*page+row][col++] = digits[localbuff[i]];

        for(col = 20, s = procName[16*page+row-4]; *s != '\0'; s++)
          scrbuff[24*page+row][col++] = *s;

        for(col = 40, s = procstate[procState[16*page+row-4]]; *s != '\0'; s++)
          scrbuff[24*page+row][col++] = *s;

        for(i = -1, x = procSize[16*page+row-4]; x > 0; x/=10)
          localbuff[++i] = x%10;
        for(col = 60; i >= 0; i--)
          scrbuff[24*page+row][col++] = digits[localbuff[i]];
      }
}

int
main(void)
{
  inittaskmgr();
  runTaskMgr();
  exit();
}
