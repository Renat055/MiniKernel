#include <stdio.h>
#include "pcb.h"

int main() {

    pcb_t p;

    p.pid = 1;
    p.burst_time = 200;
    p.remaining_time = 200;
    p.state = STATE_READY;

    printf("PID: %d\n", p.pid);
    printf("Estado: %d\n", p.state);

    return 0;
}