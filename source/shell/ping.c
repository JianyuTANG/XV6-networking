#include "types.h"
#include "network_stack.h"
#include "stat.h"
#include "user.h"
#include "nic.h"
#include "e1000.h"

int main(int argc, char **argv)
{
    if(ping(argv[1]) < 0){
        printf(1, "ifconfig command failed");
    }
    exit();
}