#include <stdio.h>
#include "metrics.h"

int main() {

    metrics_t m;

    m.total_processes = 10;
    m.completed_processes = 8;

    m.total_context_switches = 25;

    printf("Procesos totales: %d\n",
           m.total_processes);

    printf("Procesos completados: %d\n",
           m.completed_processes);

    printf("Context Switches: %d\n",
           m.total_context_switches);

    return 0;
}