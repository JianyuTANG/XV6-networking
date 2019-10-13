/*
*文件名称：history.c
*创建者：毛誉陶
*创建日期：2018/05.01
*文件描述：列出最近历史
*/

#include "types.h"
#include "user.h"
#include "fcntl.h"

void tail(int fp,int num){
	const int bufSize = 128;
	char buf[bufSize];
	char** record = malloc(sizeof(char *) * num);
	int i,length;
	int index = 0;
	int counter = 0;
	int pos = 0;
	for(i = 0;i < num;i++){
		record[i] = malloc(sizeof(char) * bufSize);
		record[i][0] = '\0';
	}
	while((length = read(fp,buf,bufSize)) > 0){
		for(i = 0;i < length;++i){
			if(buf[i] == '\n'){
				record[index][pos] = '\0';
				counter++;
				index = (index + 1) % num;
				pos = 0;
			}
			else{
				record[index][pos++] = buf[i];
			}
		}		
	}
	if(counter <= num){
		for(i = 0;i < counter;i++){
			printf(2,"%d : %s\n",(i+1),record[i]);
		}
	}
	else{
		int _index = index;
		for(i = 1;i <= num;i++){						
			printf(2,"%d : %s\n",(counter-num+i),record[_index]);
			_index = (_index + 1) % num;
		}
	}
	for(i = 0; i < num;i++){
		free(record[i]);
	}
	free(record);
}

int toNumber(char *argv, int lenLimit)
{
  int len = strlen(argv);
  int result = 0;
  if (len > lenLimit)
  {
    return -1;
  } else {
    for (int i = len - 1; i >= 0; i--)
    {
      result *= 10;
      int temp = argv[i] - '0';
      if (temp < 0 || temp > 9)
      {
        return -1;
      } else {
        result += temp;
      }
    }
  }
  return result;
}

void showHistory(int n)
{
  int fp = open("/.history", 0);
  tail(fp, n);
  close(fp);
}

void clearHistory()
{
	if(unlink("/.history") < 0){
		printf(2, "history: /.history failed to delete\n");
		return;
	}
	int fp = open("/.history", O_RDONLY | O_CREATE);
	if (fp < 0)
	{
		printf(2, "history: /.history failed to recreate\n");
		return;
	}
}

int
main(int argc, char *argv[])
{
    if (argc < 2)
    {
      showHistory(10);
    } else if (strcmp(argv[1], "-c") == 0){
			clearHistory();
		} else {
      int n = toNumber(argv[1], 32);
      if (n == -1)
      {
        printf(2, "history: numeric argument required\n");
				exit();
        return -1;
      } else {
        showHistory(n);
      }
    }
		exit();
}