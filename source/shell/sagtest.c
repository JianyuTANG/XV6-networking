#include "types.h"
#include "stat.h"
#include "user.h"

void rec(int n)
{
    int showmsg = 0;
    if (n % 1024 == 0)
        showmsg = 1;

    if (n == 0)
    {
        printf(1, "Recursion finished.\n");
        return;
    }

    if (showmsg)
    {
        printf(1, "Current recursion level: %d.\n", n);
        printf(1, "Allocating 2048 bytes of data on stack now.\n");
    }

    int a[256];
    int i;
    for (i = 0; i < 256; i++)
        a[i] = n + i;

    if (showmsg)
    {
        printf(1, "Current array location: %x.\n", a);
        printf(1, "Entering next level of recursion.\n");
    }
    rec(n - 1);
}

int main()
{
    printf(1, "================================\n");
    printf(1, "Stack auto growth test started.\n");
    printf(1, "Should use at least 12MB of stack memory.\n");
    rec(16384);
    printf(1, "Should be alive.\n");
    printf(1, "Stack auto growth test finished.\n");
    printf(1, "================================\n");
    return 0;
}