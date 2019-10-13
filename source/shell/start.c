#include <time.h>
#include "user.h"

int main(int argc, char *argv[]);

int _start(int argc, char *argv[]) {
    process_start_time = uptime();
    main(argc, argv);
    exit();
}
