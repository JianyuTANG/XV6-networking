//
// Created by gerw on 6/9/18.
//

#include <fcntl.h>
#include <stdlib.h>

extern void _init_signal();

extern int main(int argc, char *argv[]);

extern int process_start_time;

extern int uptime();

void _start(int argc, char *argv[]) {
    _init_signal();
    process_start_time = uptime();
    int ex = main(argc, argv);
    exit(ex);
}