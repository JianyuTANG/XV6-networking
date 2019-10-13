/*
*文件名称：cp.c
*创建者：程嘉梁
*创建日期：2018/04/15
*文件描述：实现文件拷贝的功能（cp命令）
*历史记录：整合自三字班方案二
*/

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"
#define BUFF_SIZE 256

#define CP_NOEXIST	0x2
#define CP_NONEW	0x4

int flag = 0;

void showInvalidOption()
{
	printf(2,"Invalid option\n");
	printf(2,"Try 'cp -h' for help\n");//三字班最初代码有误，对于此处进行修正
}

void showHelpText()
{
	printf(1,"Usage: cp [-n | -z] [src_path] [dest_path]\n");
	printf(1,"-n: Don't overwrite any file.\n");
	printf(1,"-p: Don't overwrite any file newer than source\n");	
}
  

int main(int argc, char *argv[]){
	//check the arguments
	if(argc<3 && argv[1][1]!='h')	{
		showInvalidOption();
		exit();
	}

	int now = 1;
	if(argv[1][0] == '-')
	{
		if(argv[1][2] != 0)
		{
			showInvalidOption();
			exit();
		}
		switch(argv[1][1])
		{
		case 'h':
			showHelpText();
			exit();
		case 'n':
			flag |= CP_NOEXIST;
			break;
		case 'p':
			flag |= CP_NONEW;
			break;
		default:
			showInvalidOption();
			exit();
		}
		now++;
	}
	//open source file
	int src_file = open(argv[now],O_RDONLY);
	if(src_file == -1)	{
		printf(1,"cp open source file %s failed.\n",argv[1]);
		close(src_file);
		exit();
	}
	//check whether the srouce file is a directory
	struct stat filestat,deststat;
	fstat(src_file,&filestat);
	if(filestat.type == T_DIR){
		printf(1,"cp the source file is a directory, the program cannot copy them in one time.\n");
		close(src_file);
		exit();
	}
	now++;
	if(now >= argc)
	{
		showInvalidOption();
		exit();
	}
	//check whether the second argument is ended with "/"
	char destfp[128] = {};
	strcpy(destfp,argv[now]);
	int l1 = strlen(argv[now-1]);
	int l2 = strlen(argv[now]);
	if(argv[now][l2-1]=='/')	{
		int i = 0;
		for(i = l1-1; i>=0; i--){
			if(argv[now-1][i] == '/')
				break;
		}
		i++;
		strcpy(&destfp[l2],&argv[now-1][i]);
	}
	//open the destination file
	if(flag)
	{
		int dest_file_test = open(destfp, O_WRONLY);
		if(dest_file_test != -1)
		{
			if(flag & CP_NOEXIST)
			{
				printf(1,"file exists, cp failed\n");
				close(src_file);
				close(dest_file_test);
				exit();
			}
			fstat(dest_file_test, &deststat);
			if((flag & CP_NONEW) && deststat.ctime > filestat.ctime)
			{
				printf(1,"destination is newer than source, cp failed\n");
				close(src_file);
				close(dest_file_test);
				exit();	
			}
			close(dest_file_test);
		}
	}
	int dest_file = open(destfp,O_WRONLY|O_CREATE);
	if (dest_file == -1)	{
		printf(1,"cp create file %s failed\n",destfp);
		close(src_file);
		close(dest_file);
		exit();
	}
	//copy file
	char buff[BUFF_SIZE] = {};
	int len = 0;
	while((len = read(src_file,buff,BUFF_SIZE))>0)
		write(dest_file,buff,len);
	//close files
	close(src_file);
	close(dest_file);
	exit();
}
