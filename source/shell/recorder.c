#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{
    int fd = open("record", O_WRONLY);
    if(fd<0)
        fd = open("record", O_CREATE | O_WRONLY);
    if (fd < 2) {
        printf(1, "Open file error\n");
        exit();
    }

    char buffer[1];
    int rt;
    while ((rt = read(1, buffer, 1)) >= 0) {
        if(rt == 1){
            if(write(fd, buffer, 1) != 1){
                printf(1,"Write error\n");
                break;
            }
        }
    }
    close(fd);
    exit();
}