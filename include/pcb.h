#ifndef PCB_H
#define PCB_H

#include <time.h>

// Estados del proceso
typedef enum {
    STATE_NEW,
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_TERMINATED
} process_state_t;

// Process Control Block
typedef struct {
    int pid;                    // Process ID
    int burst_time;             // Tiempo total de CPU requerido
    int remaining_time;         // Tiempo restante de ejecución
    int priority;               // Prioridad del proceso
    int arrival_time;           // Tiempo de llegada (ms)
    process_state_t state;      // Estado actual del proceso
    
    // Métricas adicionales
    int start_time;             // Tiempo de primera ejecución
    int completion_time;        // Tiempo de finalización
    int waiting_time;           // Tiempo total en espera
    int turnaround_time;        // Tiempo total desde llegada hasta finalización
    int response_time;          // Tiempo desde llegada hasta primera ejecución
    int cpu_id;                 // ID de la CPU que lo ejecuta
} pcb_t;

// Funciones para manejo de PCB
pcb_t* pcb_create(int pid, int burst_time, int arrival_time, int priority);
void pcb_destroy(pcb_t* pcb);
void pcb_print(pcb_t* pcb);
const char* pcb_state_to_string(process_state_t state);

#endif // PCB_H
