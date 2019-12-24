#include "types.h"
#include "user.h"
#include "e1000.h"
#include "nic.h"

int main(int argc, char **argv)
{
    // printf(1, "entering ifconfig\n");
    if(ifconfig(argv[1], argv[2]) < 0){
        printf(1, "ifconfig command failed");
    }
    exit();
}
