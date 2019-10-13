#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if(st.showable!=O_HIDE)
      printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      if (fmtname(buf)[0] != '.'&&st.showable!=O_HIDE){
        printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      }
    }
    break;
  }
  close(fd);
}

void  ls_1(char *path)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	 struct stat st;

	if((fd = open(path, 0)) < 0){
		printf(2, "ls: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch(st.type){
	case T_FILE:
		printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
		break;

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
		printf(1, "ls: path too long\n");
		break;
		}
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if(de.inum == 0)
			continue;
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		if(stat(buf, &st) < 0){
			printf(1, "ls: cannot stat %s\n", buf);
			continue;
		}
        printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
	}
    break;
  }
  close(fd);
}
int compare_x(char *c1,char* c2)
{
	int len1, len2;
	len1 = strlen(c1);
	len2 = strlen(c2);
	if(len1 != len2)
	{
		return 0;
	}
	int i;
	for(i = 0; i < len1; i++)
	{
		if(c1[i] != c2[i])
		{
			return 0;
		}
	}
	return 1;
}

int
main(int argc, char *argv[])
{
  int i;
  if(argc < 2){
    ls(".");
    exit();
  }
  else
  {
	  if(compare_x(argv[1], "-a"))
	  {
		  ls_1(".");
		  exit();
	  }
	  else
	  {
		  ls(argv[1]);
		  exit();
	  }
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit();
}

