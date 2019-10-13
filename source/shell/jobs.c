#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "jobsconst.h"

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

int main(void)
{
    int fd;
    
    fd = open(JOBS_FILENAME, O_RDONLY);
    if(fd >= 0) 
    {
        //printf(1, "ok: open file succeed\n");
    } 
    else 
    {
        printf(1, "error: open file failed\n");
        exit();
    }
    char* line = " ";
    int id = 1;
    int size;
    while(1)//读入每行
    {
        size = jobs_readline(fd, line, 100);
        if(size > 100 || size < 0)
            break;
        char res[20];
        char slash[] = " ";
        int pos = 0;
                  
        pos = partition(line, res, pos);
        int pid = atoi(res);
            
        int state = getstate(pid);
            
        pos = partition(line, res, pos);
        printf(1, res);//输出名称
        printf(1,slash);

        switch(state)
        {
            case UNUSED: printf(1,"UNUSED\n"); break;
            case EMBRYO: printf(1,"EMBRYO\n"); break;
            case SLEEPING: printf(1,"SLEEPING\n"); break;
            case RUNNABLE: printf(1,"RUNNABLE\n"); break;
            case ZOMBIE: printf(1,"ZOMBIE\n"); break;
            default: printf(1,"STOPPED\n"); break;
        }           

        id++;       
    }
    //printf(1, "read all\n");
    close(fd);
    return exit();
}