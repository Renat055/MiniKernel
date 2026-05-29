#include "pcb.h"
#include "queue.h"
#include "metrics.h"
#include "sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef _WIN32
    #include <windows.h>
    #define usleep(x) Sleep((x)/1000)
#else
    #include <unistd.h>
#endif

#define QUANTUM 100  // Quantum en milisegundos

// Estructura para pasar datos a cada hilo CPU
typedef struct {
    int cpu_id;
    ready_queue_t* ready_queue;
    metrics_t* metrics;
} cpu_args_t;

// Simular la ejecución de un proceso por un quantum
void execute_process(pcb_t* process, int cpu_id, int quantum) {
    int execution_time = (process->remaining_time < quantum) ? 
                         process->remaining_time : quantum;
    
    // Simular ejecución (dormir por el tiempo de ejecución)
    usleep(execution_time * 1000);  // Convertir ms a microsegundos
    
    process->remaining_time -= execution_time;
    
    printf("[CPU%d] Ejecutando P%d | Restante: %d ms | Estado: RUNNING\n",
           cpu_id, process->pid, process->remaining_time);
}

// Función que ejecuta cada hilo CPU
void* cpu_thread_function(void* arg) {
    cpu_args_t* args = (cpu_args_t*)arg;
    int cpu_id = args->cpu_id;
    ready_queue_t* ready_queue = args->ready_queue;
    metrics_t* metrics = args->metrics;
    
    printf("[CPU%d] Iniciada\n", cpu_id);
    fflush(stdout);
    
    while (sync_is_running()) {
        pcb_t* process = NULL;
        
        // Intentar obtener un proceso de la cola
        pthread_mutex_lock(&ready_queue->mutex);
        
        if (ready_queue->front != NULL) {
            // Obtener proceso del frente de la cola
            queue_node_t* node = ready_queue->front;
            process = node->process;
            ready_queue->front = node->next;
            if (ready_queue->front == NULL) {
                ready_queue->rear = NULL;
            }
            ready_queue->size--;
            free(node);
        }
        
        pthread_mutex_unlock(&ready_queue->mutex);
        
        // Si no hay proceso, dormir un poco y continuar
        if (process == NULL) {
            usleep(10000);  // 10ms
            continue;
        }
        
        // Marcar tiempo de inicio si es la primera vez
        if (process->start_time == -1) {
            process->start_time = sync_get_current_time();
        }
        
        // Cambiar estado a RUNNING
        process->state = STATE_RUNNING;
        process->cpu_id = cpu_id;
        
        // Context switch
        metrics_increment_context_switches(metrics);
        
        // Ejecutar el proceso por un quantum
        execute_process(process, cpu_id, QUANTUM);
        fflush(stdout);
        
        // Actualizar tiempo global
        sync_increment_time(QUANTUM);
        
        // Verificar si el proceso terminó
        if (process->remaining_time <= 0) {
            process->state = STATE_TERMINATED;
            process->completion_time = sync_get_current_time();
            
            printf("[CPU%d] P%d TERMINADO | Tiempo total: %d ms\n",
                   cpu_id, process->pid, process->burst_time);
            fflush(stdout);
            
            // Agregar a métricas
            metrics_add_completed_process(metrics, process);
            
            // Liberar PCB
            pcb_destroy(process);
        } else {
            // Proceso no terminó, volver a la cola (Round Robin)
            process->state = STATE_READY;
            queue_enqueue(ready_queue, process);
            
            printf("[CPU%d] P%d -> Ready Queue (preemption)\n", cpu_id, process->pid);
            fflush(stdout);
        }
    }
    
    printf("[CPU%d] Finalizada\n", cpu_id);
    fflush(stdout);
    return NULL;
}

// Crear y lanzar hilos CPU
pthread_t* create_cpu_threads(int n_cpus, ready_queue_t* ready_queue, metrics_t* metrics) {
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * n_cpus);
    
    for (int i = 0; i < n_cpus; i++) {
        cpu_args_t* args = (cpu_args_t*)malloc(sizeof(cpu_args_t));
        args->cpu_id = i;
        args->ready_queue = ready_queue;
        args->metrics = metrics;
        
        if (pthread_create(&threads[i], NULL, cpu_thread_function, args) != 0) {
            perror("Error al crear hilo CPU");
            exit(1);
        }
    }
    
    return threads;
}

// Esperar a que todos los hilos CPU terminen
void join_cpu_threads(pthread_t* threads, int n_cpus) {
    for (int i = 0; i < n_cpus; i++) {
        pthread_join(threads[i], NULL);
    }
}
