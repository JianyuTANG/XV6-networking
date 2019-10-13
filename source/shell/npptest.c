#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
    int *a = 0;
    printf(1, "================================\n");
    printf(1, "Null pointer protection test started.\n");
    printf(1, "This process should be killed and produce no more messages.\n");
    printf(1, "Dereferencing null pointer! Value is %d.\n", *a);
    printf(1, "The process should be killed but is alive.\n");
    printf(1, "Test failed.\n");
    return 0;
}