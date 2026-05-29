#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>
#include <stdbool.h>

// Variables globales de sincronización y control
extern pthread_mutex_t global_mutex;
extern pthread_cond_t process_available;
extern bool simulation_running;
extern int current_time_ms;

// Funciones de sincronización
void sync_init(void);
void sync_destroy(void);
void sync_signal_process_available(void);
void sync_wait_for_process(pthread_mutex_t* queue_mutex);
void sync_stop_simulation(void);
bool sync_is_running(void);
int sync_get_current_time(void);
void sync_increment_time(int ms);

#endif // SYNC_H
