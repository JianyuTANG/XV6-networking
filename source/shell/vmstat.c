#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
    int n=nfpgs();
    printf(1, "Used physical memory:%dBytes\n", 224*1024*1024-4096*n);
    printf(1, "Avalible physical memory:%dBytes\n", 4096*n);
    n=getsharedpages();
    printf(1, "Used shared memory:%dBytes\n",4096*n);
    showproc();
    printf(1, "\n");
    exit();
}
