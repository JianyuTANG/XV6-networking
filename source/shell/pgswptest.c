#include "types.h"
#include "stat.h"
#include "user.h"

void rec(int n)
{
    if (n == 0)
    {
        printf(1, "Recursion exit.\n");
        return;
    }
    else if (n % 50 == 0 || n < 100)
        printf(1, "Recursion level %d\n", n);

    int array[16384];
    int i;
    for (i = 0; i < 16384; i++)
        (void)(array[i] = i);

    rec(n - 1);
}

int main()
{
    printf(1, "================================\n");
    printf(1, "Page swapping test started.\n");

    // This is a magic number, we only have a few swapping memory.
    // If the level is set too big, will use out swapfile.
    // If too small, will not trigger page swapping function.
    rec(532);

    printf(1, "Page swapping test finished.\n");
    printf(1, "================================\n");
    return 0;
}