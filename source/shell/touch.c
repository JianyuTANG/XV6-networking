/*
*文件名称：touch.c
*创建者：程嘉梁
*创建日期：2018/04/14
*文件描述：实现新建文件的功能（touch命令）
*历史记录：整合自三字班方案二
*/
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf(1, "Please input this command as [touch file_name1 file_name2 ...]\n");
	}

	int i, count = 0;
	for (i = 1; i < argc; i++) {		
		int fd;
		// 测试文件是否存在
		if ((fd = open(argv[i], O_RDONLY)) < 0) {
			// 文件不存在就创建它
			fd = open(argv[i], O_CREATE|O_RDONLY);
			count++;
		}
		close(fd);
	}

	printf(1, "%d file(s) created, %d file(s) skiped.\n", count, argc - 1 - count);

	exit();
}
