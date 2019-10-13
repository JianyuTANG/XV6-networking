/*
*文件名称：more.c
*创建者：程嘉梁
*创建日期：2018/05/07
*文件描述：实现按页阅读文件的功能（more命令）
*/

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define LINE_LEN 50//每行最长长度
#define PAGE_LINES 5//每页行数
#define PAGE_MAX 300//最大页数

void more(int fp)
{
	int i,j,length;
	const int bufSize = 1024;
	char buf[bufSize];
	int page_capacity[PAGE_MAX];
	char c[6];
	int current_page,current_line,line_num,nextpos;

	//初始化
	current_page=current_line=line_num=nextpos=0;
	j=0;
	char *show=(char*)malloc(bufSize*sizeof(char));
	for(i=0;i<PAGE_MAX;i++)
	{
		page_capacity[i]=0;
	}
	
	length=read(fp,buf,sizeof(buf));
	for(i=0;i<length&&line_num<PAGE_LINES;i++)
	{
		if(buf[i]=='\n'&&current_line<LINE_LEN)
		{
			show[j]='\n';
			current_line=0;
			page_capacity[current_page]++;
			line_num++;	
			j++;				
		}
		else if(current_line>=LINE_LEN)
		{
			current_line=0;		
			show[j]='\n';	
			j++;
			line_num++;
			if(buf[i]=='\n')
			{
				page_capacity[current_page]++;					
			}
			else
			{
				i--;
			}			
		}
		else
		{
			show[j]=buf[i];
			j++;
			current_line++;
			page_capacity[current_page]++;
		}
	}
	show[j]='\0';
	j=0;
	printf(1,"------------------------------\n");
	printf(1,"%s",show);
	printf(1,"------------------------------\n");

	while(1)
	{
		gets(c,5);
		switch(c[0])
		{
		case 'd':
			current_page++;
			nextpos=0;
			for(i=0;i<current_page;i++)
			{
				nextpos+=page_capacity[i];
			}
			lseek(fp,nextpos,0);
			if((length=read(fp,buf,sizeof(buf)))<=0)//读到最后一页
			{
				
				current_page--;
				nextpos=0;
				for(i=0;i<current_page;i++)
				{
					nextpos+=page_capacity[i];
				}
				
			}
			break;
		case 'u':
			current_page--;
			nextpos=0;
			if(current_page<0)//读到第一页
			{
				current_page=0;
			}
			for(i=0;i<current_page;i++)
			{
				nextpos+=page_capacity[i];
			}
			break;
		case 'z':
			nextpos=-1;
			break;		
		default:
			printf(1,"input 'd': Read next page.\n");
			printf(1,"input 'u': Read previous page.\n");
			printf(1,"input 'z': Exit.\n");	
			nextpos=-2;
			break;
			
		}
		
		if(nextpos==-1)
		{
			free(show);
			return;
		}
		if(nextpos==-2)
		{
			continue;
		}
		
		lseek(fp,nextpos,0);
		length=read(fp,buf,sizeof(buf));
		current_line=line_num=0;
		j=0;
		page_capacity[current_page]=0;
		for(i=0;i<length&&line_num<PAGE_LINES;i++)
		{
			if(buf[i]=='\n'&&current_line<LINE_LEN)
			{
				show[j]='\n';
				current_line=0;
				page_capacity[current_page]++;
				line_num++;	
				j++;				
			}
			else if(current_line>=LINE_LEN)
			{
				current_line=0;		
				show[j]='\n';	
				j++;
				line_num++;
				if(buf[i]=='\n')
				{
					page_capacity[current_page]++;					
				}
				else
				{
					i--;
				}			
			}
			else
			{
				show[j]=buf[i];
				j++;
				current_line++;
				page_capacity[current_page]++;
			}
		}
		show[j]='\0';
		j=0;
		printf(1,"------------------------------\n");
		printf(1,"%s",show);
		printf(1,"------------------------------\n");
	}
	

	free(show);
	
}

int main(int argc,char* argv[])
{
	//判断参数
	if(argc!=2)
	{
		printf(1, "please input the command as [more dest_file]\n");
		exit();
	}
	
	//打开文件
	int fp;
	if((fp=open(argv[1],O_RDONLY))<0)
	{
		printf(1,"more : cannot open %s\n",argv[1]);
		exit();	
	}
	
	//判断文件状态是否为文件夹
	struct stat st;
	fstat(fp, &st);
	if (st.type == T_DIR)
	{
		printf(1, "the file is a directory!\n");
		exit();
	}
	
	more(fp);
	close(fp);
	
	exit();
	
}
