#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

// Crear una nueva cola de procesos listos
ready_queue_t* queue_create(void) {
    ready_queue_t* queue = (ready_queue_t*)malloc(sizeof(ready_queue_t));
    if (!queue) {
        perror("Error al crear la cola");
        return NULL;
    }
    
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    
    return queue;
}

// Destruir la cola y liberar memoria
void queue_destroy(ready_queue_t* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    
    queue_node_t* current = queue->front;
    while (current) {
        queue_node_t* next = current->next;
        free(current);
        current = next;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    
    free(queue);
}

// Agregar un proceso a la cola (al final)
void queue_enqueue(ready_queue_t* queue, pcb_t* process) {
    if (!queue || !process) return;
    
    queue_node_t* new_node = (queue_node_t*)malloc(sizeof(queue_node_t));
    if (!new_node) {
        perror("Error al crear nodo de cola");
        return;
    }
    
    new_node->process = process;
    new_node->next = NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->rear == NULL) {
        // Cola vacía
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        // Agregar al final
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
    
    queue->size++;
    process->state = STATE_READY;
    
    pthread_mutex_unlock(&queue->mutex);
}

// Remover y retornar el proceso al frente de la cola
pcb_t* queue_dequeue(ready_queue_t* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Esperar mientras la cola esté vacía
    while (queue->front == NULL) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    queue_node_t* node = queue->front;
    pcb_t* process = node->process;
    
    queue->front = node->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    queue->size--;
    free(node);
    
    pthread_mutex_unlock(&queue->mutex);
    
    return process;
}

// Verificar si la cola está vacía
int queue_is_empty(ready_queue_t* queue) {
    if (!queue) return 1;
    
    pthread_mutex_lock(&queue->mutex);
    int empty = (queue->size == 0);
    pthread_mutex_unlock(&queue->mutex);
    
    return empty;
}

// Obtener el tamaño de la cola
int queue_size(ready_queue_t* queue) {
    if (!queue) return 0;
    
    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    
    return size;
}

// Imprimir el contenido de la cola
void queue_print(ready_queue_t* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    
    printf("Ready Queue (size: %d): ", queue->size);
    queue_node_t* current = queue->front;
    while (current) {
        printf("P%d ", current->process->pid);
        current = current->next;
    }
    printf("\n");
    
    pthread_mutex_unlock(&queue->mutex);
}
