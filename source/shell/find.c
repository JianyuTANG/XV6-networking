/*
*文件功能：文件查找
*历史来源：四字班方案一代码
*整合作者：赵哲晖
*整合时间：2018/05/09
*/

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "date.h"

typedef enum
{
	type_error,
	type_name,
	type_iname,
	type_size,
	type_empty,
	type_cmin,
	type_ctime
} 	
find_type;    //查询命令类型

char* getFnameFromPath(char* path,char* fname)
{
	int i,j;
	i = 0;
	int len = strlen(path);
	for(i = 0; i < len; i++)
	{
		for(j = i; j  < len; j++)
		{
			if((j == 0 && path[j] == '.')  ||  (path[j] == '/'))
			{
				break;
			}
		}
		if(j == len)
		{
			break;
		}
	}
	if(i == len)
		return ".";

	int Len = (len - i);
	//char fname[100];
	for(j = 0;j < Len; j++)
	{
		fname[j] = path[i+j];
	}
	fname[Len] = 0;
	return fname;
}

/*
*函数功能：动态规划实现含通配符字符串匹配
*作者：赵哲晖
*时间：2018/05/21
*/
int match(char * name, char * fname)
{
	int dm[15][15];
	int m = strlen(name);
	int n = strlen(fname);
	dm[0][0] = 1;
	for(int i = 1; fname[i-1] == '*'; i++)
		dm[i][0] = 1;
	for(int i = 1; i <= n; i++)
	{
		for(int j = 1; j <= m; j++)
		{
			if(fname[i-1] == '?')
				dm[i][j] = dm[i-1][j-1];
			else if(fname[i-1] == '*')
			{
				dm[i][j] = (dm[i-1][j] || dm[i-1][j-1] || dm[i][j-1]);
			}	
			else
				dm[i][j] = (dm[i-1][j-1] && (fname[i-1] == name[j-1]));
		}
	}
	return dm[n][m];
}
	
int ignore_case(char *name, char* fname)
{
	//忽略大小写不考虑模糊匹配
	/*int len1 = strlen(name);
	int len2 = strlen(fname);
	if(len1 != len2)
	{
		return 0;
	}
	for(int i = 0; i < len1; i++)
	{
		if(name[i] != fname[i] && name[i] != fname[i] - 32 && name[i] != fname[i] + 32)
		{
			return 0;
		}
	}
	return 1;*/ 

	//忽略大小写考虑模糊匹配
	int dm[15][15];
	int m = strlen(name);
	int n = strlen(fname);
	dm[0][0] = 1;
	for(int i = 1; fname[i-1] == '*'; i++)
		dm[i][0] = 1;
	for(int i = 1; i <= n; i++)
	{
		for(int j = 1; j <= m; j++)
		{
			if(fname[i-1] == '?')
				dm[i][j] = dm[i-1][j-1];
			else if(fname[i-1] == '*')
			{
				dm[i][j] = (dm[i-1][j] || dm[i-1][j-1] || dm[i][j-1]);
			}	
			else
				dm[i][j] = ( dm[i-1][j-1] && ( (fname[i-1] == name[j-1]) || (fname[i-1] == name[j-1] + 32) || (fname[i-1] == name[j-1] - 32) ) );
		}
	}
	return dm[n][m];
}

int compare(char *c1,char* c2)
{
	int len1, len2;
	len1 = strlen(c1);
	len2 = strlen(c2);
	if(len1 != len2)
	{
		return -1;
	}
	int i;
	for(i = 0; i < len1; i++)
	{
		if(c1[i] != c2[i] && c2[i] != '?')
		{
			return -1;
		}
	}
	return 0;
}
int compare1(char *c1,char* c2)
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

/*char*
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

	switch(st.type)
	{
	case T_FILE:
		printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
		break;

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
		{
			printf(1, "ls: path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		while(read(fd, &de, sizeof(de)) == sizeof(de))
		{
			if(de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if(stat(buf, &st) < 0)
			{
				printf(1, "ls: cannot stat %s\n", buf);
				continue;
			}
			printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
		}
		break;
	}
	close(fd);
}
*/
/*
*函数功能：uint转化为实时时钟
*作者：赵哲晖
*时间：2018/05/15
*/
rtcdate* timestampToDate(uint stamp)
{
  uint ret = 946684800; //utc+0 2000/1/1 0:0:0 946684800
  uint YEAR_SEC = 31536000;
  uint DAY_SEC = 86400;
  stamp -= ret;
  int years = 0;
  //get years
  while(1)
  {
    if(years % 4 == 0)
    {
      if(stamp >= YEAR_SEC + DAY_SEC)
      {
        stamp = stamp - (YEAR_SEC + DAY_SEC);
        years++;
      }
      else
        break;
    }
    else
    {
      if(stamp >= YEAR_SEC)
      {
        stamp -= YEAR_SEC;
        years++;
      }
      else
        break;
    }
  }
  //get month
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if(years != 0 && years % 4 == 0)
    days[1] = 29;
  int month = 1;
  while(1)
  {
    int temp = stamp - days[month-1] * DAY_SEC;
    if(temp >= 0)
    {
      stamp = temp;
      month++;
    }  
    else
      break;
  }
  //get days
  int day = 1;
  while(stamp >= DAY_SEC)
  {
    stamp -= DAY_SEC;
    day++;
  }
  //get hours
  int hour = 0;
  while(stamp >= 3600)
  {
    stamp -= 3600;
    hour++;
  }
  //get minutes
  int minute = 0;
  while(stamp >= 60)
  {
    stamp -= 60;
    minute++;
  }
  int second = stamp;
  rtcdate * date = malloc(sizeof(rtcdate));
  date->year = years + 2000;
  date->month = month;
  date->day = day;
  date->hour = hour;
  date->minute = minute;
  date->second = second;
  return date;
}

//按照文件名查找文件
void find_name(char *path, char *fname, int type, int deleteOrNot)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	if((fd = open(path, 0)) < 0)
	{
		printf(2, "ls: cannot open %s\n", path);
		return;
	}
	if(fstat(fd, &st) < 0){
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}
	switch(st.type)
	{
	case T_FILE:
		{
			char name[DIRSIZ+1];
			if(type == type_name)
			{
				if(match(getFnameFromPath(path,name),fname) != 0)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			else if(type == type_iname)
			{
				if(ignore_case(getFnameFromPath(path,name),fname) != 0)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			break;
		}
	

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
		{
			printf(1, "ls: path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf+strlen(buf);
		if(*(p-1) == '.')
			*p++ = '/';
		while(read(fd, &de, sizeof(de)) == sizeof(de))
		{
			if(de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if(stat(buf, &st) < 0){
				printf(1, "ls: cannot stat %s\n", buf);
				continue;
			}
			char name[DIRSIZ+1];
			if(type == type_name)
			{
				if(match(getFnameFromPath(buf,name),fname) != 0)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			else if(type == type_iname)
			{
				if(ignore_case(getFnameFromPath(buf,name),fname) != 0)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			getFnameFromPath(buf,name);
			if((st.type == 1) && (compare(name,".") != 0) && (compare(name,"..") != 0) )
			{
				//printf(1,"%s %d %d", getFnameFromPath(buf,name), compare(getFnameFromPath(buf,name),"."), compare(getFnameFromPath(buf,name),".."));
				//printf(0,"2");
				char bufnext[512];
				strcpy(bufnext,buf);
				char * q = bufnext + strlen(bufnext);
				*q = '/';
				find_name(bufnext, fname, type, deleteOrNot);
			}
		}
		break;
	}
	close(fd);
}

//根据输入大小查找文件
void find_size(char *path, int MoreOrLess, int size, int deleteOrNot)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	if((fd = open(path, 0)) < 0)
	{
		printf(2, "ls: cannot open %s\n", path);
		return;
	}
	if(fstat(fd, &st) < 0){
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}
	switch(st.type)
	{
	case T_FILE:
		{
			char name[DIRSIZ+1];
			if( (st.size <= size && MoreOrLess == 0) || (st.size >= size && MoreOrLess == 1) || st.size == size)
			{
				getFnameFromPath(path, name);
				if(compare1(name, ".") != 1 && compare1(name, "..") != 1)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			break;
		}
	

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
		{
			printf(1, "ls: path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf+strlen(buf);
		if(*(p-1) == '.')
			*p++ = '/';
		while(read(fd, &de, sizeof(de)) == sizeof(de))
		{
			if(de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if(stat(buf, &st) < 0){
				printf(1, "ls: cannot stat %s\n", buf);
				continue;
			}
			char name[DIRSIZ+1];
			if((st.size <= size && MoreOrLess == 0) || (st.size >= size && MoreOrLess == 1) || st.size == size)
			{
				getFnameFromPath(buf,name);
				if(compare1(name, ".") != 1 && compare1(name, "..") != 1)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			getFnameFromPath(buf,name);
			if((st.type == 1) && (compare(name,".") != 0) && (compare(name,"..") != 0) )
			{
				char bufnext[512];
				strcpy(bufnext,buf);
				char * q = bufnext + strlen(bufnext);
				*q = '/';
				find_size(bufnext, MoreOrLess, size, deleteOrNot);
			}
		}
		break;
	}
	close(fd);
}

//根据时间查找文件
void find_time(char *path, int time, int type, int now, int deleteOrNot)
{
	
	int timeDifference;//时间差

	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	if((fd = open(path, 0)) < 0)
	{
		printf(2, "ls: cannot open %s\n", path);
		return;
	}
	if(fstat(fd, &st) < 0){
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}
	switch(st.type)
	{
	case T_FILE:
		{
			char name[DIRSIZ+1];
			getFnameFromPath(path, name);
			timeDifference = now - st.ctime;
			if(type == type_cmin)
			{
				if(timeDifference <= time*60 && (compare1(name, ".") != 1) && compare1(name, "..") != 1 )
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			else if(type == type_ctime)
			{
				if(timeDifference <= time*3600*24 && (compare1(name, ".") != 1) && compare1(name, "..") != 1)
				{
					rtcdate * date = timestampToDate(st.ctime);
					printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
					free(date);
					if(deleteOrNot == 1)
					{
						unlink(name);
					}
				}
			}
			break;
		}
	

	case T_DIR:
		{
			if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
			{
				printf(1, "ls: path too long\n");
				break;
			}
			strcpy(buf, path);
			p = buf+strlen(buf);
			if(*(p-1) == '.')
				*p++ = '/';
			while(read(fd, &de, sizeof(de)) == sizeof(de))
			{
				if(de.inum == 0)
					continue;
				memmove(p, de.name, DIRSIZ);
				p[DIRSIZ] = 0;
				if(stat(buf, &st) < 0)
				{
					printf(1, "ls: cannot stat %s\n", buf);
					continue;
				}
				char name[DIRSIZ+1];
				getFnameFromPath(buf,name);
				timeDifference = now - st.ctime;
				if(type == type_cmin)
				{
					if(timeDifference <= time*60 && (compare1(name, ".") != 1) && compare1(name, "..") != 1)
					{
						rtcdate * date = timestampToDate(st.ctime);
						printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
						free(date);
						if(deleteOrNot == 1)
						{
							unlink(name);
						}
					}
				}
				else if(type == type_ctime)
				{
					if(timeDifference <= time*3600*24 && (compare1(name, ".") != 1) && compare1(name, "..") != 1)
					{
						rtcdate * date = timestampToDate(st.ctime);
						printf(1, "%s path:%s%s  size:%d  time: %d/%d/%d %d:%d:%d\n", name, path, name, st.size,date->year,date->month,date->day,date->hour,date->minute,date->second);
						free(date);
						if(deleteOrNot == 1)
						{
							unlink(name);
						}	
					}
				}
				
				if((st.type == 1) && (compare(name,".") != 0) && (compare(name,"..") != 0) )
				{
					char bufnext[512];
					strcpy(bufnext,buf);
					char * q = bufnext + strlen(bufnext);
					*q = '/';
					find_time(bufnext, time, type, now, deleteOrNot);
				}
			}

			break;
		}
		
	}
	close(fd);

}

/*
*函数功能：返回查询命令类型
*作者：赵哲晖
*时间：2018/05/21
*/
int getFindType(char * type)
{
	if(!strcmp(type,"-name"))
		return type_name;
	else if(!strcmp(type,"-iname"))
		return type_iname;
	else if(!strcmp(type,"-size"))
		return type_size;
	else if(!strcmp(type,"-empty"))
		return type_empty;
	else if(!strcmp(type,"-cmin"))
		return type_cmin;
	else if(!strcmp(type,"-ctime"))
		return type_ctime;
	else
		return type_error;

}

int main(int argc, char *argv[])
{
	//int t = gettimestamp();
	//printf(2,"%d",t);
	if(argc < 3)
	{
		//printf(1, "please input the filename you want to find...\n");
		printf(1, "Error: too few arguments\n");
		exit();
	}
	if(open(argv[1], 0) < 0)
	{
		printf(2, "Error: cannot open directory %s\n", argv[1]);
		exit();
	}
	/*int i;
	for(i=1; i<argc; i++)
	ls(argv[i]);*/
	int type = getFindType(argv[2]);
	if(!type)
	{
		printf(1, "Error: expression error\n");
		exit();
	}
	int len = 0;
	int time = 0;
	int deleteOrNot = 0;//查找到文件后是否删除，0表示不删除，1表示删除
	switch(type)
	{
	case type_name:
		if(argc < 4)
		{
			printf(1, "Error: too few arguments\n");
			exit();
		}
		if(strlen(argv[3]) > 14)
		{
			printf(1, "Error: filename too long\n");
			exit();
		}
		if(argc == 4)
		{
			deleteOrNot = 0;
			find_name(argv[1],argv[3], type_name, deleteOrNot);
		}
		else if(argc == 5)
		{
			if(compare1(argv[4], "delete") == 0)
			{
				printf(1, "Error: Instruction is not clear\n");
				exit();
			}
			else
			{
				deleteOrNot = 1;
				find_name(argv[1],argv[3], type_name, deleteOrNot);
			}
		}
		break;
	case type_iname:
		if(argc < 4)
		{
			printf(1, "Error: too few arguments\n");
			exit();
		}
		if(strlen(argv[3]) > 14)
		{
			printf(1, "Error: filename too long\n");
			exit();
		}
		if(argc == 4)
		{
			deleteOrNot = 0;
			find_name(argv[1], argv[3], type_iname, deleteOrNot);
		}
		else if(argc == 5)
		{
			if(compare1(argv[4], "delete") == 0)
			{
				printf(1, "Error: Instruction is not clear\n");
				exit();
			}
			else
			{
				deleteOrNot = 1;
				find_name(argv[1],argv[3], type_name, deleteOrNot);
			}
		}
		break;
	case type_size:
		if(argc < 4)
		{
			printf(1, "Error: too few arguments\n");
			exit();
		}
		if(strlen(argv[3]) > 14)
		{
			printf(1, "Error: filename too long\n");
			exit();
		}
		int MoreOrLess;//判断寻找的是大于等于该size还是小于等于该size,大于则MoreOrLess = 1, 小于则=0;
		if(argv[3][0] == '+')
		{
			MoreOrLess = 1;
		}
		else if(argv[3][0] == '-')
		{
			MoreOrLess = 0;
		}
		else
		{
			printf(1, "Error: The size is not clear\n");
			exit();
		}

		len = strlen(argv[3]);
		int size = 0;
		for(int i = 1; i < len; i++)
		{
			if(argv[3][i] >= '0' && argv[3][i] <= '9')
			{
				size = size * 10 + argv[3][i] - '0';
			}
			else
			{
				printf(1, "Error: The size is not a integer\n");
				exit();
				break;
			}
		}
		if(argc == 4)
		{
			deleteOrNot = 0;
			find_size(argv[1], MoreOrLess, size, deleteOrNot);
		}
		else if(argc == 5)
		{
			if(compare1(argv[4], "delete") == 0)
			{
				printf(1, "Error: Instruction is not clear\n");
				exit();
			}
			else
			{
				deleteOrNot = 1;
				find_size(argv[1], MoreOrLess, size, deleteOrNot);
			}
		}
		break;


	case type_empty:
		if(argc == 3)
		{
			deleteOrNot = 0;
			find_size(argv[1],0, 0, deleteOrNot);
		}
		else if(argc == 4)
		{
			if(compare1(argv[3], "delete") == 0)
			{
				printf(1, "Error: Instruction is not clear\n");
				exit();
			}
			else
			{
				deleteOrNot = 1;
				find_size(argv[1], 0, 0, deleteOrNot);
			}
		}
		break;


	case type_cmin:
		if(argc < 4)
		{
			printf(1, "Error: too few arguments\n");
			exit();
		}
		if(strlen(argv[3]) > 14)
		{
			printf(1, "Error: filename too long\n");
			exit();
		}

		len = strlen(argv[3]);
		time = 0;
		for(int i = 0; i < len; i++)
		{
			if(argv[3][i] >= '0' && argv[3][i] <= '9')
			{
				time = time * 10 + argv[3][i] - '0';
			}
			else
			{
				printf(1, "Error: The time is not a integer\n");
				exit();
				break;
			}
		}
		int now = gettimestamp();//当前时间戳

		if(argc == 4)
		{
			deleteOrNot = 0;
			find_time(argv[1], time, type_cmin, now, deleteOrNot);
		}
		else if(argc == 5)
		{
			if(compare1(argv[4], "delete") == 0)
			{
				printf(1, "Error: Instruction is not clear\n");
			}
			else
			{
				deleteOrNot = 1;
				find_time(argv[1], time, type_cmin, now, deleteOrNot);
			}
		}
		break;

	case type_ctime:
		if(argc < 4)
		{
			printf(1, "Error: too few arguments\n");
			exit();
		}
		if(strlen(argv[3]) > 14)
		{
			printf(1, "Error: filename too long\n");
			exit();
		}

		len = strlen(argv[3]);
		time = 0;
		for(int i = 0; i < len; i++)
		{
			if(argv[3][i] >= '0' && argv[3][i] <= '9')
			{
				time = time * 10 + argv[3][i] - '0';
			}
			else
			{
				printf(1, "Error: The time is not a integer\n");
				exit();
				break;
			}
		}
		now = gettimestamp();//当前时间戳
		if(argc == 4)
		{
			find_time(argv[1], time, type_ctime, now, deleteOrNot);
		}
		else if(argc == 5)
		{
			if(compare1(argv[4], "delete") == 0)
			{
				deleteOrNot = 0;
				printf(1, "Error: Instruction is not clear\n");
			}
			else
			{
				deleteOrNot = 1;
				find_time(argv[1], time, type_cmin, now, deleteOrNot);
			}
		}
		break;
	default:
		printf(1, "Not finished yet\n");
		break;
	}
	exit();
}


