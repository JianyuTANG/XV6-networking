#include "types.h"
#include "stat.h"
#include "user.h"

int a = 0;

int main()
{
    printf(1, "================================\n");
    printf(1, "Copy on write test started.\n");
    // Simple fork test (exit first).
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    if (fork() == 0)
    {
        printf(1, "[Child ] Current free pages: %d\n", nfpgs());
        printf(1, "[Child ] Current value of a: %d\n", a);
        a = 1;
        printf(1, "[Child ] Current free pages: %d\n", nfpgs());
        printf(1, "[Child ] Current value of a: %d\n", a);
        exit();
    }
    printf(1, "[Parent] Sleeping.\n");
    sleep(100);
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    printf(1, "[Parent] Waiting.\n");
    wait();
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    a = 2;
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);

    // Recursive fork test.
    a = 0;
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);

    if (fork() == 0)
    {
        printf(1, "[Child1] Current free pages: %d\n", nfpgs());
        printf(1, "[Child1] Current value of a: %d\n", a);
        if (fork() == 0)
        {
            printf(1, "[Child2] Current free pages: %d\n", nfpgs());
            printf(1, "[Child2] Current value of a: %d\n", a);
            a = 3;
            printf(1, "[Child2] Current free pages: %d\n", nfpgs());
            printf(1, "[Child2] Current value of a: %d\n", a);
            exit();
        }
        printf(1, "[Child1] Current free pages: %d\n", nfpgs());
        printf(1, "[Child1] Current value of a: %d\n", a);
        wait();
        exit();
    }
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    wait();
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    a = 4;
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);

    // Simple fork test (wait first).
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    if (fork() == 0)
    {
        printf(1, "[Child ] Sleeping\n");
        sleep(100);
        printf(1, "[Child ] Current free pages: %d\n", nfpgs());
        printf(1, "[Child ] Current value of a: %d\n", a);
        a = 1;
        printf(1, "[Child ] Current free pages: %d\n", nfpgs());
        printf(1, "[Child ] Current value of a: %d\n", a);
        exit();
    }
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    printf(1, "[Parent] Waiting.\n");
    wait();
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);
    a = 2;
    printf(1, "[Parent] Current free pages: %d\n", nfpgs());
    printf(1, "[Parent] Current value of a: %d\n", a);

    printf(1, "Copy on write test finished.\n");
    printf(1, "================================\n");
    return 0;
}