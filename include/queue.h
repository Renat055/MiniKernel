#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"
#include <pthread.h>

// Nodo de la cola
typedef struct queue_node {
    pcb_t* process;
    struct queue_node* next;
} queue_node_t;

// Cola de procesos listos (Ready Queue)
typedef struct {
    queue_node_t* front;
    queue_node_t* rear;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
} ready_queue_t;

// Funciones de la cola
ready_queue_t* queue_create(void);
void queue_destroy(ready_queue_t* queue);
void queue_enqueue(ready_queue_t* queue, pcb_t* process);
pcb_t* queue_dequeue(ready_queue_t* queue);
int queue_is_empty(ready_queue_t* queue);
int queue_size(ready_queue_t* queue);
void queue_print(ready_queue_t* queue);

#endif // QUEUE_H
