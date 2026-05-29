#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex;

int contador = 0;

void* increment(void* arg) {

    pthread_mutex_lock(&mutex);

    contador++;

    pthread_mutex_unlock(&mutex);

    return NULL;
}