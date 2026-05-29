#include <stdio.h>

#define QUANTUM 100

int main() {

    int remaining_time = 250;

    remaining_time -= QUANTUM;

    printf("Remaining: %d\n", remaining_time);

    return 0;
}