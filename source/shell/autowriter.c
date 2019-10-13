#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{
    int fd = open("writerout", O_WRONLY);
    if (fd < 0)
        fd = open("writerout", O_CREATE | O_WRONLY);
    if (fd < 2) {
        printf(1, "Open file error\n");
        exit();
    }
    while(1){
        sleep(100);
        write(fd, "I am working...\n", 17);
    }
    close(fd);
    exit();
}