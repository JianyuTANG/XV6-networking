#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main()
{
    char cwd[100];
    getcwd(cwd);
    printf(1,"%s\n",cwd);
    exit();
}
