#include "metrics.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Crear estructura de métricas
metrics_t* metrics_create(void) {
    metrics_t* metrics = (metrics_t*)malloc(sizeof(metrics_t));
    if (!metrics) {
        perror("Error al crear métricas");
        return NULL;
    }
    
    metrics->total_processes = 0;
    metrics->completed_processes = 0;
    metrics->avg_waiting_time = 0.0;
    metrics->avg_turnaround_time = 0.0;
    metrics->avg_response_time = 0.0;
    metrics->throughput = 0.0;
    metrics->total_context_switches = 0;
    
    metrics->completed_capacity = 100;
    metrics->completed = (pcb_t**)malloc(sizeof(pcb_t*) * metrics->completed_capacity);
    
    pthread_mutex_init(&metrics->mutex, NULL);
    
    return metrics;
}

// Destruir estructura de métricas
void metrics_destroy(metrics_t* metrics) {
    if (!metrics) return;
    
    pthread_mutex_lock(&metrics->mutex);
    
    // Liberar procesos completados
    for (int i = 0; i < metrics->completed_processes; i++) {
        if (metrics->completed[i]) {
            free(metrics->completed[i]);
        }
    }
    free(metrics->completed);
    
    pthread_mutex_unlock(&metrics->mutex);
    pthread_mutex_destroy(&metrics->mutex);
    
    free(metrics);
}

// Agregar un proceso completado para cálculo de métricas
void metrics_add_completed_process(metrics_t* metrics, pcb_t* process) {
    if (!metrics || !process) return;
    
    pthread_mutex_lock(&metrics->mutex);
    
    // Expandir array si es necesario
    if (metrics->completed_processes >= metrics->completed_capacity) {
        metrics->completed_capacity *= 2;
        metrics->completed = (pcb_t**)realloc(metrics->completed, 
                                              sizeof(pcb_t*) * metrics->completed_capacity);
    }
    
    // Copiar el PCB
    pcb_t* copy = (pcb_t*)malloc(sizeof(pcb_t));
    memcpy(copy, process, sizeof(pcb_t));
    
    metrics->completed[metrics->completed_processes] = copy;
    metrics->completed_processes++;
    
    pthread_mutex_unlock(&metrics->mutex);
}

// Calcular métricas de rendimiento
void metrics_calculate(metrics_t* metrics, int elapsed_time_ms) {
    if (!metrics) return;
    
    pthread_mutex_lock(&metrics->mutex);
    
    if (metrics->completed_processes == 0) {
        pthread_mutex_unlock(&metrics->mutex);
        return;
    }
    
    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;
    
    for (int i = 0; i < metrics->completed_processes; i++) {
        pcb_t* p = metrics->completed[i];
        
        // Calcular tiempos
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;
        p->response_time = p->start_time - p->arrival_time;
        
        total_waiting += p->waiting_time;
        total_turnaround += p->turnaround_time;
        total_response += p->response_time;
    }
    
    // Promedios
    metrics->avg_waiting_time = total_waiting / metrics->completed_processes;
    metrics->avg_turnaround_time = total_turnaround / metrics->completed_processes;
    metrics->avg_response_time = total_response / metrics->completed_processes;
    
    // Throughput (procesos por segundo)
    if (elapsed_time_ms > 0) {
        metrics->throughput = (metrics->completed_processes * 1000.0) / elapsed_time_ms;
    }
    
    pthread_mutex_unlock(&metrics->mutex);
}

// Imprimir métricas
void metrics_print(metrics_t* metrics) {
    if (!metrics) return;
    
    pthread_mutex_lock(&metrics->mutex);
    
    printf("\n========== METRICAS DE RENDIMIENTO ==========\n");
    printf("Total de procesos generados:    %d\n", metrics->total_processes);
    printf("Procesos completados:            %d\n", metrics->completed_processes);
    printf("Context switches totales:        %d\n", metrics->total_context_switches);
    printf("Tiempo promedio de espera:       %.2f ms\n", metrics->avg_waiting_time);
    printf("Tiempo promedio de respuesta:    %.2f ms\n", metrics->avg_response_time);
    printf("Tiempo promedio de turnaround:   %.2f ms\n", metrics->avg_turnaround_time);
    printf("Throughput:                      %.4f procesos/seg\n", metrics->throughput);
    printf("=============================================\n\n");
    
    // Detalles de cada proceso
    printf("Detalles de procesos completados:\n");
    printf("PID\tArrival\tBurst\tStart\tCompletion\tWaiting\tResponse\tTurnaround\tCPU\n");
    printf("---\t-------\t-----\t-----\t----------\t-------\t--------\t----------\t---\n");
    
    for (int i = 0; i < metrics->completed_processes; i++) {
        pcb_t* p = metrics->completed[i];
        printf("%d\t%d\t%d\t%d\t%d\t\t%d\t%d\t\t%d\t\tCPU%d\n",
               p->pid, p->arrival_time, p->burst_time, p->start_time,
               p->completion_time, p->waiting_time, p->response_time,
               p->turnaround_time, p->cpu_id);
    }
    printf("\n");
    
    pthread_mutex_unlock(&metrics->mutex);
}

// Incrementar contador de context switches
void metrics_increment_context_switches(metrics_t* metrics) {
    if (!metrics) return;
    
    pthread_mutex_lock(&metrics->mutex);
    metrics->total_context_switches++;
    pthread_mutex_unlock(&metrics->mutex);
}
