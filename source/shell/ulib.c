#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

char
getc_from_stdin()
{
  int c;
  read(0, &c, 1);
  return c;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

int jobs_readline(int fd, char* line, int n)
{  
  strcpy(line, "\0"); 
  int i = 0;  
  for(i = 0; i < n; i++)
  {    
    char buf[2];    
    int m = read(fd, buf, 1);
    //printf(1,buf);	
    if(m == 0)
    {//read over      
    return -2;    
    }    
    else if(m < 0)
    {
      //read error      
      return -1;    
    }    
    if(buf[0]=='\n')
    {      
      break;    
    }    
    char bufarray[2];		
    bufarray[0] = buf[0];		
    bufarray[1] = '\0';		
    strcpy(line + strlen(line), bufarray);  
  }  
  strcpy(&line[strlen(line)],"\0");  
  if(i <= n)
  {
    //return the line lenth    
    return i;  
  }  
  else return n+1;//n is too small 
}

int partition(char *src, char *par, int pos)
{
	int i, j;
	i = pos;

	//取到第一个非空格字符 
	while (src[i] == ' ')
	{
		++i;
	}

	if (src[i] != '\0')
	{
		j = 0;
		while ((src[i] != '\0') && (src[i] != ' '))
		{
			par[j] = src[i];
			++i;
			++j;
		}
		par[j] = '\0';

		return i;
	}
	else
	{
		return -1;
	}
}