#ifndef METRICS_H
#define METRICS_H

#include "pcb.h"
#include <pthread.h>

// Estructura para almacenar métricas globales
typedef struct {
    int total_processes;
    int completed_processes;
    double avg_waiting_time;
    double avg_turnaround_time;
    double avg_response_time;
    double throughput;
    int total_context_switches;
    
    // Sincronización
    pthread_mutex_t mutex;
    
    // Array de procesos completados para cálculo de métricas
    pcb_t** completed;
    int completed_capacity;
} metrics_t;

// Funciones de métricas
metrics_t* metrics_create(void);
void metrics_destroy(metrics_t* metrics);
void metrics_add_completed_process(metrics_t* metrics, pcb_t* process);
void metrics_calculate(metrics_t* metrics, int elapsed_time_ms);
void metrics_print(metrics_t* metrics);
void metrics_increment_context_switches(metrics_t* metrics);

#endif // METRICS_H
