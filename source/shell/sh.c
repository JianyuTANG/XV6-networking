// Shell.
//sh.c文件的修改者：毛誉陶,江俊广,赵哲晖,丁润语
#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "jobsconst.h"
#include "history.h"
#include "console.h"
#include "stat.h"
#include "fs.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10
#define COMMANDNUM 45
char *command[COMMANDNUM] = { "cat", "echo", "forktest", "grep", "init", "kill", "ln", "ls",
"mkdir", "rm", "sh", "stressfs", "usertests", "wc", "zombie", "taskmgr", "cowtest", "lalloctest", 
"npptest", "sagtest", "pgswptest", "shmtest", "vmstat", "find", "bi", "vim", "mv", "touch", "cp",
"head", "tail", "splice", "history", "shutdown", "login", "more", "date", "pwd", "delete", "refresh", 
"showdeled", "jerry", "timetest", "stdtests", "rename" };


struct cmd {
  int type;
};

struct execcmd {//执行程序命令
  int type;
  char *argv[MAXARGS];//程序参数的开头 的数组
  char *eargv[MAXARGS];//程序参数的末尾 的数组
  //[ argv[0], eargv[0] ) 构成了程序的第一个参数
};

struct redircmd {//重定向命令
  int type;
  struct cmd *cmd;
  char *file;//文件名字符串的开头
  char *efile;//文件名字符串的结尾
  int mode;//打开文件的模式
  int fd;//文件描述符
};

struct pipecmd {//管道命令
  int type;
  struct cmd *left;//管道输入命令
  struct cmd *right;//管道的删除命令
};

struct listcmd {//多个进程（前面命令执行成功与否都去执行后面的指令）
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {//后台进程（无用户交互）
  int type;
  struct cmd *cmd;
};

struct history hs;

void initHistory(struct history* hs);
void addHistory(struct history* hs,char* cmd);
void getHistory(struct history* hs);
void setHistory(char* cmd);

void findCommand(char * buf);

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);
void runcmd(struct cmd*);

//cache for last cmd
char lastcmd[2][100];
int lastcmd_pointer;

//write a command to JOBS_FILENAME
void 
recordcmd(int pid,char* cmdname)
{
  int cachefd;
  int buffersize = 1024;
  int haveread = 0;
  char buf[1024];

  cachefd = open(JOBS_FILENAME,O_RDONLY);
  if (cachefd >= 0) {
    haveread = read(cachefd,buf,buffersize);
    buf[haveread] = 0;
    close(cachefd);
  }

  cachefd = open(JOBS_FILENAME, O_WRONLY);
  if (cachefd < 0)
    cachefd = open(JOBS_FILENAME, O_CREATE | O_WRONLY);
  if (cachefd < 0){
    printf(2, "Cannot open ");
    printf(2,JOBS_FILENAME);
    printf(2,"\n");
  }
  else{
    write(cachefd,buf,haveread);
    printf(cachefd, "%d %s\n", pid, cmdname);
    close(cachefd);
  }
  return;
}
//Run bg
void 
bg()
{
  int forkid;
  if(lastcmd[lastcmd_pointer][0] == 0){
    printf(2, "No current job\n");
    return;
  }
 
  printf(1, "%s &\n", lastcmd[lastcmd_pointer]);

  forkid = fork1();
  if (forkid == 0)
  {
    reparent(forkid, 2);
    runcmd(parsecmd(lastcmd[lastcmd_pointer]));
  }
  else
  {
    recordcmd(forkid,lastcmd[lastcmd_pointer]);
  }
exit();
}

//Run fg
//Important:this function has not been completed
void
fg(char* s)
{
  printf(2,s);
  int fgid;
  fgid = 0;
  while('0' <= *s && *s <= '9')
    fgid = fgid*10 + *s++ - '0';

  reparent(fgid,2);
  wait();
  exit();
}
// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  int forkid;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit();

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC://执行某个程序
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
    int i;//转换为绝对路径
    for(i = 0;ecmd->argv[0][i];i++);
    char* ncommand = malloc((i+1)*sizeof(char));
    ncommand[0] = '/';
    for(i = 0;ecmd->argv[0][i];i++)
    {
        ncommand[i+1] = ecmd->argv[0][i];
    }
//<<<<<<< final_shell
    exec(ncommand, ecmd->argv);
//=======
//    if (ecmd->argv[0][0] == 'b' && ecmd->argv[0][1] == 'g'){
//      bg();
//    }
//    else if (ecmd->argv[0][0] == 'f' && ecmd->argv[0][1] == 'g'){
//      fg(ecmd->argv[0] + 3);
//    }
//    else{
//      exec(ecmd->argv[0], ecmd->argv);
//    }
//>>>>>>> final_shell
    printf(2, "exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR://重定向
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      printf(2, "open %s failed\n", rcmd->file);
      exit();
    }//保证cmd运行前，需要的文件rcmd->file已经被打开
    runcmd(rcmd->cmd);
    break;

  case LIST: 
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)//子进程执行left命令
      runcmd(lcmd->left);
    wait();//父进程等待子进程结束后
    runcmd(lcmd->right);//执行right命令
    break;

  case PIPE://管道命令
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)//pipe返回一个管道的输入和输出描述符p[0],p[1]
      panic("pipe");
    if(fork1() == 0){//创建一个子进程
      close(1);//关闭原先的标准输出
      dup(p[1]);//将p[1]绑定到标准输出
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);//pcmd->left的输出流向了p[1]      
    }
    if(fork1() == 0){//再创建一个子进程
      close(0);//关闭原先的标准输入
      dup(p[0]);//将p[0]绑定到标准输入
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);//pcmd->right输入来自p[0]
    }
    close(p[0]);
    close(p[1]);//在父进程中也需要关闭对管道的读写控制
    wait();//等待子进程返回
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    forkid = fork1();
    if(forkid == 0){
      reparent(getpid(),2);
      runcmd(bcmd->cmd);
    }
    else
      recordcmd(forkid,lastcmd[1-lastcmd_pointer]);
    break;
  }
  exit();
}

void
renewline(char* buf, int nbuf)
{
  int j;
  for(j = 0; buf[j]; j++){
    clearc();
  }
  for(j = 0; buf[j]; j++){//将缓冲区buf中的内容输出到显示区
    insertc(buf[j]);
  }
  memset(buf, 0, nbuf);//然后清空缓冲区
}

int
getcmd(char *buf, int nbuf)//从键盘读入命令到buf中，最长不能超过nbuf-1
{
  int i, j;
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  char c;
  for(i=0; i+1 < nbuf; ){ 
    c = getc_from_stdin();
    if( ( i==0 || buf[i-1] == 0 ) && c == 0 )//若输入的为首字符或者此前已经输入了空格
      continue;//则可以忽略多余的空格（适用性最好的实现方式应该时对输入的buf进行分词，此处简化处理，只过滤多余的空格）
    if(c+256 == 0xE2){//key_up
      clearc(); // 去掉上键
      if (hs.length != H_NUMBER && hs.curcmd == 0){ 
        renewline(buf, nbuf);
        i = 0;
        continue;
      }      
      if (hs.curcmd == (hs.lastcmd + 1) % H_NUMBER){
        renewline(buf, nbuf);
        i = 0;
        continue;
      }
      for(j = 0; buf[j]; j++){
        clearc();
      } 
      hs.curcmd = (hs.curcmd + H_NUMBER - 1) % H_NUMBER;
      for(j = 0; hs.record[hs.curcmd][j]; j++ )
        insertc(hs.record[hs.curcmd][j]);
      i = 0;
      strcpy(buf, hs.record[hs.curcmd]);
      continue;
    }
    if (c+256 == 0xE3){//key_down
      clearc();
      if (hs.curcmd != hs.lastcmd){
        hs.curcmd = (hs.curcmd + 1) % H_NUMBER;
      }
      for(j = 0; buf[j]; j++){
        clearc();
      }
      for(j = 0; hs.record[hs.curcmd][j]; j++ )
        insertc(hs.record[hs.curcmd][j]);
      i = 0;
      strcpy(buf, hs.record[hs.curcmd]);
      continue;
    }
    if(c == 9)//tab
    {
      clearc();
      buf[i] = '*';
      findCommand(buf);
      memset(buf, 0, nbuf);
      i = 0;
      continue;
    }
    if(c == '\n' || c == '\r')
      break;
    buf[i++] = c;
  }
  // char s[5]; s[0] = '0'+i;s[2]='\n';s[3]=0; s[1] = buf[0]+'0'; 
  // printf(2, s);
  
  buf[i] = '\0';
  if(buf[0] == 0) // EOF
    return 0;
  return 0;
}

int 
processCmdLine(char* line)//处理一行shell命令
{
  if(strlen(line) <= 0) return -1;
  if(strlen(line) >= 3 && line[0] == 'c' && line[1] == 'd' && line[2] == ' '){//如果line中命令是cd
    // Chdir must be called by the parent, not the child.
    line[strlen(line)] = 0;  // chop \n
    if(chdir(line+3) < 0)
      printf(2, "cannot cd %s\n", line+3);
  }else{//line中命令不是cd，
    if(fork1() == 0){//创建子进程
      runcmd(parsecmd(line));//执行line中的命令，执行完后会自动销毁子进程
    }else{
      wait();//父进程等待子进程返回后需要继续运行
    }
  }
  return 0;
}

int 
executeShellFile(char* filename){
//从filename中读入shell命令，并执行
  int fd = open(filename, O_RDONLY);
  if(fd < 0)
    return fd;
  char buf[128];//读入字符缓冲区
  int n, i, p = 0;
  const int MAX_LINE_LENGTH = 128;
  char line[MAX_LINE_LENGTH];//一行命令，设置最长为128个字符
  while((n = read(fd, buf, 128)) > 0){//从文件中不断读入字符到缓存区
    for(i = 0; i < n; i++){
      if(buf[i] == '\n'){//如果读入了换行符
        line[p] = '\0';//则可以执行该行命令了
        p = 0;
        processCmdLine(line);
      }else{//如果还没有读入换行符
        if(p < MAX_LINE_LENGTH){
          line[p++] = buf[i];//从缓冲区中读取到line中
        }else{
          printf(2,"error:单行命令长度不能超过128\n");
        }
      }
    }
  }
  close(fd);
  return 0;
}

void findCommand(char * buf)  //自动补全
{
  printf(1,"\n");
  int count = 0;
  char * p = 0;
  char * q = 0;
  for(int i = 0; i < COMMANDNUM; i++)
  {
    p = buf;
    q = command[i];
    while((*p)!=0 && (*q)!=0)
    {
      if(*p == *q)
      {
        p++; q++;
      }  
      else
        break;
      if(*p == '*')
      {
        printf(2,"%s ", command[i]);
        count++;
        break;
      }
    }
  }
  if(!count)
    printf(1,"No command found");
  printf(1,"\n");
}

int
main(int argc, char *argv[])
{
  if(argc <= 1){//如果没有参数，说明是创建一个shell终端
    static char buf[100];
    int fd;
    initHistory(&hs);
    getHistory(&hs);

    // Ensure that three file descriptors are open.
    while((fd = open("console", O_RDWR)) >= 0){
      if(fd >= 3){
        close(fd);
        break;
      }
    }
    //clear processInfo
    unlink(JOBS_FILENAME);

    // Read and run input commands.
    while(getcmd(buf, sizeof(buf)) >= 0){
      strcpy(lastcmd[lastcmd_pointer], buf);
      lastcmd[lastcmd_pointer][strlen(lastcmd[lastcmd_pointer]) - 1] = 0;
      lastcmd_pointer = 1 - lastcmd_pointer;
      if (buf[0] == '!' && buf[1] == '!')
      {
        strcpy(buf, hs.record[(hs.lastcmd - 1) % H_NUMBER]);
      } else {
        addHistory(&hs, buf);
        setHistory(buf);
      }
      processCmdLine(buf);//处理buf中存储的命令
    }
    exit();
  }else{//如果有参数，目前只接受以文件名作为参数
    int i;
    for(i=1; i<argc; i++){
      executeShellFile(argv[i]);//用shell执行参数对应的文件
    }
    exit();
  }
  return 0;
}


void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;//left指向自己所要执行的命令
  cmd->right = right;//right指向下一个要指向的命令
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;//从*ps字符串的开头开始寻找
  while(s < es && strchr(whitespace, *s))//忽略多余的空格
    s++;
  if(q)
    *q = s;//q记录有意义字符串的开头位置
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|'://一个字符长度的符号
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){//两个字符长度的符号
      ret = '+';//返回'+'表示读入的是">>"
      s++;
    }
    break;
  default://如果读入的是一个普通字符，则
    ret = 'a';//返回'a'
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))//则以空格或者符号作为分隔，
      s++;//读入整个普通字符串
    break;
  }
  if(eq)
    *eq = s;//*eq记录读入的有意义字符串结尾的位置

  while(s < es && strchr(whitespace, *s))//忽略到s后多余的空格
    s++;
  *ps = s;//*ps记录下一次开始搜索的位置
  return ret;//返回读入的字符串类型
}

int
peek(char **ps, char *es, char *toks)//忽略字符次*ps中的空格，返回其中首个字符是否属于toks
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))//首先忽略掉所有的空格
    s++;
  *ps = s;//*ps指向首个非空字符
  return *s && strchr(toks, *s);//判断*s是否属于toks
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)//将命令字符串转化为命令结构体
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);//es指向字符串s的尾部
  cmd = parseline(&s, es);//从字符次*s得到命令结构体
  peek(&s, es, "");//忽略s后所有的空格
  if(s != es){//如果没有处理完s中的所有内容
    printf(2, "leftovers: %s\n", s);
    panic("syntax");//说明有语法错误
  }
  nulterminate(cmd);///将cmd中所有字符串的结尾设置为0,以便后续程序正确的处理
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)//读取一行字符串*ps
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);//从*ps开头读取一组按照管道连接的命令
  while(peek(ps, es, "&")){//若*ps中首个非空字符为&
    gettoken(ps, es, 0, 0);//则读入这个&字符
    cmd = backcmd(cmd);//命令在后台运行
  }
  if(peek(ps, es, ";")){//若*ps中首个非空字符为;
    gettoken(ps, es, 0, 0);//则读入这个;字符
    cmd = listcmd(cmd, parseline(ps, es));//递归构造命令列表
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)//从*ps开头读取一组按照管道连接的命令
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){//若ps中首个非空字符为|
    gettoken(ps, es, 0, 0);//则读入这个|字符
    cmd = pipecmd(cmd, parsepipe(ps, es));//构造以管道连接的一组命令
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
//从*ps开头读取一个重定向命令，将cmd重定向后返回
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){//如果*ps开头是字符<,>
    tok = gettoken(ps, es, 0, 0);//读入该字符('<','>',">>")
    if(gettoken(ps, es, &q, &eq) != 'a')//然后读入其后的文件名，[q,eq)为文件名字符串
      panic("missing file for redirection");
    switch(tok){
    case '<'://重定向标准输入
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>'://重定向标准输出，覆盖原文件
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >> 重定向标准输出，追加到原文件
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)//从*ps开头读取以'('')'包围的命令
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);//括号中的命令将会新开一个子shell顺序执行
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);//子shell的输入和输出有可能重定向到父shell
  return cmd;
}


struct cmd*
parseexec(char **ps, char *es)//读取一个可执行命令
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))//如果*ps开头有由括号构成的block
    return parseblock(ps, es);//则返回由该block构成的命令

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);//即使是*ps的开头也可能出现重定向命令
  while(!peek(ps, es, "|)&;")){//如果*ps的开头不是|)&;
    if((tok=gettoken(ps, es, &q, &eq)) == 0)//如果已经到达了*ps的尾部
      break;//则退出循环
    if(tok != 'a')//如果读入的不是一个普通字符串
      panic("syntax");//则为语法错误
    cmd->argv[argc] = q;//不断读入程序的参数
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)//如果超过程序参数的上限
      panic("too many args");//则报错
    ret = parseredirs(ret, ps, es);//任意参数之间都可以有重定向命令
  }
  cmd->argv[argc] = 0;//通过判断argv[]中第几个字符为空，可以得到程序参数的个数
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)//将cmd中所有字符串的结尾设置为0,以便后续程序正确的处理
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}

void initHistory(struct history* hs){
  hs->length = 1;
  hs->curcmd = 0;
  hs->lastcmd = 0;
}

void addHistory(struct history* hs, char* cmd){
  if (cmd[0] == '\n')
    return;
  int l = strlen(cmd);
  int last = hs->lastcmd;
  strcpy(hs->record[last], cmd);
  if (hs->record[last][l - 1] == '\n')
    hs->record[last][l - 1] = '\0';
  last = (last + 1) % H_NUMBER;
  hs->record[last][0] = '\0';
  hs->lastcmd = last;
  hs->curcmd = last;
  if (hs->length < H_NUMBER)
    hs->length++;
}

void getHistory(struct history* hs){ 
  const int bufSize = 128;
  int fp = open("/.history", O_RDONLY | O_CREATE);
  int i, length, pos;

  char buf[bufSize];
  char tmp[bufSize];

  pos = 0;
  initHistory(hs);
  while ((length = read(fp, buf, bufSize)) > 0){
    for (i = 0; i < length; ++i){
      if (buf[i] == '\n'){
        tmp[pos] = '\0';
        addHistory(hs, tmp);
        pos = 0;
      }
      else{
        tmp[pos++] = buf[i];
      }
    }
  }

  close(fp);
}

void setHistory(char* cmd){
    int fp = open("/.history", O_WRONLY | O_CREATE | O_ADD);
    write(fp, cmd, strlen(cmd));
    write(fp, "\n", 1);
    close(fp);
}
