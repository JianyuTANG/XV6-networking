#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
    printf(1, "================================\n");
    printf(1, "Lazy allocation test started.\n");

    printf(1, "Current free pages: %d.\n", nfpgs());

    // Should use 1024 pages.
    int *arr = malloc(0x400000);
    printf(1, "Current free pages after \"allocation\": %d.\n", nfpgs());

    int i;
    for (i = 0; i < 0x100000; i++)
    {
        arr[i] = i;
        if (i % 0x10000 == 0)
            printf(1, "Current free pages when i = %d : %d.\n", i, nfpgs());
    }

    free(arr);

    printf(1, "Current free pages after free(): %d.\n", nfpgs());

    printf(1, "Lazy allocation test finished.\n");
    printf(1, "================================\n");

    return 0;
}