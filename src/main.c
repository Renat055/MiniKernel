#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include "pcb.h"
#include "queue.h"
#include "metrics.h"
#include "sync.h"

#ifdef _WIN32
    #include <windows.h>
    #define usleep(x) Sleep((x)/1000)
#else
    #include <unistd.h>
#endif

// Configuración del sistema
#define N_CPUS 4
#define MAX_PROCESSES 20
#define MIN_BURST_TIME 50
#define MAX_BURST_TIME 500
#define MIN_SLEEP_TIME 100
#define MAX_SLEEP_TIME 300

// Variables globales
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t process_available = PTHREAD_COND_INITIALIZER;
bool simulation_running = true;
int current_time_ms = 0;

// Declaraciones de funciones externas (cpu.c)
extern pthread_t* create_cpu_threads(int n_cpus, ready_queue_t* ready_queue, metrics_t* metrics);
extern void join_cpu_threads(pthread_t* threads, int n_cpus);

// Estructura para el generador de procesos
typedef struct {
    ready_queue_t* ready_queue;
    metrics_t* metrics;
    int max_processes;
} generator_args_t;

// Implementación de funciones de PCB
pcb_t* pcb_create(int pid, int burst_time, int arrival_time, int priority) {
    pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
    if (!pcb) {
        perror("Error al crear PCB");
        return NULL;
    }
    
    pcb->pid = pid;
    pcb->burst_time = burst_time;
    pcb->remaining_time = burst_time;
    pcb->priority = priority;
    pcb->arrival_time = arrival_time;
    pcb->state = STATE_NEW;
    pcb->start_time = -1;
    pcb->completion_time = 0;
    pcb->waiting_time = 0;
    pcb->turnaround_time = 0;
    pcb->response_time = 0;
    pcb->cpu_id = -1;
    
    return pcb;
}

void pcb_destroy(pcb_t* pcb) {
    if (pcb) {
        free(pcb);
    }
}

void pcb_print(pcb_t* pcb) {
    if (!pcb) return;
    printf("Process P%d: burst=%d, remaining=%d, state=%s\n",
           pcb->pid, pcb->burst_time, pcb->remaining_time,
           pcb_state_to_string(pcb->state));
}

const char* pcb_state_to_string(process_state_t state) {
    switch (state) {
        case STATE_NEW: return "NEW";
        case STATE_READY: return "READY";
        case STATE_RUNNING: return "RUNNING";
        case STATE_WAITING: return "WAITING";
        case STATE_TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// Implementación de funciones de sincronización
void sync_init(void) {
    pthread_mutex_init(&global_mutex, NULL);
    pthread_cond_init(&process_available, NULL);
    simulation_running = true;
    current_time_ms = 0;
}

void sync_destroy(void) {
    pthread_mutex_destroy(&global_mutex);
    pthread_cond_destroy(&process_available);
}

void sync_signal_process_available(void) {
    pthread_cond_broadcast(&process_available);
}

void sync_wait_for_process(pthread_mutex_t* queue_mutex) {
    pthread_cond_wait(&process_available, queue_mutex);
}

void sync_stop_simulation(void) {
    pthread_mutex_lock(&global_mutex);
    simulation_running = false;
    pthread_mutex_unlock(&global_mutex);
    pthread_cond_broadcast(&process_available);
}

bool sync_is_running(void) {
    pthread_mutex_lock(&global_mutex);
    bool running = simulation_running;
    pthread_mutex_unlock(&global_mutex);
    return running;
}

int sync_get_current_time(void) {
    pthread_mutex_lock(&global_mutex);
    int time = current_time_ms;
    pthread_mutex_unlock(&global_mutex);
    return time;
}

void sync_increment_time(int ms) {
    pthread_mutex_lock(&global_mutex);
    current_time_ms += ms;
    pthread_mutex_unlock(&global_mutex);
}

// Generador de número aleatorio en un rango
int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Hilo generador de procesos
void* process_generator_thread(void* arg) {
    generator_args_t* args = (generator_args_t*)arg;
    ready_queue_t* ready_queue = args->ready_queue;
    metrics_t* metrics = args->metrics;
    int max_processes = args->max_processes;
    
    printf("[GENERATOR] Iniciado\n");
    
    for (int i = 0; i < max_processes; i++) {
        // Crear nuevo proceso
        int burst_time = random_range(MIN_BURST_TIME, MAX_BURST_TIME);
        int arrival_time = sync_get_current_time();
        int priority = random_range(1, 10);
        
        pcb_t* process = pcb_create(i, burst_time, arrival_time, priority);
        
        if (process) {
            printf("[GENERATOR] Creado P%d | Burst: %d ms | Arrival: %d ms\n",
                   process->pid, process->burst_time, process->arrival_time);
            fflush(stdout);
            
            // Agregar a la cola de procesos listos
            queue_enqueue(ready_queue, process);
            
            // Actualizar métricas
            pthread_mutex_lock(&metrics->mutex);
            metrics->total_processes++;
            pthread_mutex_unlock(&metrics->mutex);
        }
        
        // Dormir un tiempo aleatorio antes de crear el siguiente proceso
        if (i < max_processes - 1) {  // No dormir después del último proceso
            int sleep_time = random_range(MIN_SLEEP_TIME, MAX_SLEEP_TIME);
            usleep(sleep_time * 1000);
        }
    }
    
    printf("[GENERATOR] Finalizado - %d procesos creados\n", max_processes);
    free(args);
    return NULL;
}

// Función principal
int main(void) {
    printf("========================================\n");
    printf("       MiniKernel - Simulador de SO    \n");
    printf("========================================\n");
    printf("Configuracion:\n");
    printf("  - CPUs: %d\n", N_CPUS);
    printf("  - Procesos: %d\n", MAX_PROCESSES);
    printf("  - Quantum: 100 ms\n");
    printf("  - Algoritmo: Round Robin\n");
    printf("========================================\n\n");
    
    // Inicializar semilla aleatoria
    srand(time(NULL));
    
    // Inicializar sincronización
    sync_init();
    
    // Crear estructuras de datos
    ready_queue_t* ready_queue = queue_create();
    metrics_t* metrics = metrics_create();
    
    if (!ready_queue || !metrics) {
        fprintf(stderr, "Error al inicializar estructuras\n");
        return 1;
    }
    
    // Crear hilos CPU
    printf("Iniciando %d CPUs...\n", N_CPUS);
    pthread_t* cpu_threads = create_cpu_threads(N_CPUS, ready_queue, metrics);
    
    // Crear hilo generador de procesos
    printf("Iniciando generador de procesos...\n\n");
    pthread_t generator_thread;
    generator_args_t* gen_args = (generator_args_t*)malloc(sizeof(generator_args_t));
    gen_args->ready_queue = ready_queue;
    gen_args->metrics = metrics;
    gen_args->max_processes = MAX_PROCESSES;
    
    pthread_create(&generator_thread, NULL, process_generator_thread, gen_args);
    
    // Esperar a que el generador termine
    pthread_join(generator_thread, NULL);
    
    // Esperar a que todos los procesos se completen
    printf("\nEsperando a que todos los procesos terminen...\n");
    while (metrics->completed_processes < MAX_PROCESSES) {
        usleep(100000);  // 100ms
    }
    
    // Detener simulación
    printf("\nDeteniendo simulacion...\n");
    sync_stop_simulation();
    
    // Señalar a todas las CPUs que hay cambios
    pthread_mutex_lock(&ready_queue->mutex);
    pthread_cond_broadcast(&ready_queue->not_empty);
    pthread_mutex_unlock(&ready_queue->mutex);
    
    // Esperar a que los hilos CPU terminen
    join_cpu_threads(cpu_threads, N_CPUS);
    
    // Calcular y mostrar métricas
    int total_time = sync_get_current_time();
    metrics_calculate(metrics, total_time);
    metrics_print(metrics);
    
    // Limpiar recursos
    queue_destroy(ready_queue);
    metrics_destroy(metrics);
    free(cpu_threads);
    sync_destroy();
    
    printf("Simulacion completada exitosamente.\n");
    printf("Tiempo total de simulacion: %d ms\n", total_time);
    
    return 0;
}
