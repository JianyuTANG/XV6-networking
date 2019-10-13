/*
*文件名称：login.c
*创建者：程嘉梁
*创建日期：2018/05/06
*文件描述：实现Xv6登录口令操作
*/

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#define MAXLEN 20//用户名和密码最长长度

//检查输入是否正确
int checkpasswd(char *user,char *passwd)
{
  int i,j,n;
  char ipasswd[MAXLEN];
  char iuser[MAXLEN];
  char buf[1024];
  
  int fd=open("passwd",O_RDONLY);
  while((n = read(fd,buf,sizeof(buf)))>0)
  {
  	i=j=0;
  	while(i<n&&buf[i]!='\n')
  		iuser[j++]=buf[i++];
  	if(i==n) break;
  	iuser[j]='\0';
  	i++;
  	j=0;
   	while(i<n&&buf[i]!='\n')
  		ipasswd[j++]=buf[i++];
  	if(i==n) break;
  	ipasswd[j]='\0';
  }
  
  close(fd);
  if(!strcmp(user,iuser)&&!strcmp(passwd,ipasswd))
		return 1;
	else
		return 0;
		
}

//用于设置用户名和密码
int setpasswd()
{
  char ipasswd[MAXLEN];
  char iuser[MAXLEN];
  int fd;
  fd = open("passwd", O_WRONLY|O_CREATE);
  if(fd<0)
  {
    printf(2, "create passwd file failed!");
    return 0; 
  }


	printf(1, "set your user login:");
	gets(iuser,MAXLEN);
	printf(1,"set your password:");
	gets(ipasswd,MAXLEN);

	write(fd,iuser,strlen(iuser));
	write(fd,ipasswd,strlen(ipasswd));
	close(fd);
	return 0;
}

int main(int argc, char *argv[])
{
  char user[MAXLEN];
  char passwd[MAXLEN];
  char *shargv[] = {"sh",0};
  int pid,wpid;

	int n;
  char buf[1024];
  int fd = open("passwd", O_RDONLY);
  if((n= read(fd, buf, sizeof(buf))) <= 0)
  {
    setpasswd();
  }

  while(1)
	{
    printf(1, "user login:");
    gets(user,MAXLEN);
    printf(1,"password:");
    gets(passwd,MAXLEN);
    user[strlen(user)-1]='\0';
    passwd[strlen(passwd)-1]='\0';
    if(checkpasswd(user,passwd))
		{
      pid = fork();
      if(pid < 0)
			{
        printf(1, "init: fork failed\n");
        exit();
      }
      if(pid == 0)
			{
        exec("sh", shargv);
        printf(1, "init: exec sh failed\n");
        exit();
      }
      while((wpid=wait()) >= 0 && wpid != pid)
        printf(1, "zombie!\n");
    }
    else
		{
      printf(1,"wrong user or password!\n\n");
    }

  }

	return 0;
}

