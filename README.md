# MiniKernel

MiniKernel es una simulación básica de un sistema operativo desarrollada en lenguaje C utilizando programación concurrente con `pthreads`. El proyecto implementa conceptos fundamentales de Sistemas Operativos como administración de procesos, planificación de CPU, sincronización entre hilos y métricas de rendimiento.

## Funcionamiento

El sistema crea múltiples procesos representados mediante estructuras PCB (`Process Control Block`). Estos procesos son almacenados en una Ready Queue y posteriormente ejecutados por CPUs simuladas mediante hilos.

El scheduler utiliza el algoritmo **Round Robin**, asignando un quantum fijo de tiempo a cada proceso. Cuando un proceso consume su quantum y aún no termina, vuelve a la cola READY para continuar posteriormente su ejecución.

Para evitar condiciones de carrera en el acceso concurrente a la Ready Queue y variables compartidas, se implementaron mecanismos de sincronización utilizando `mutex` y variables de condición.

Además, el sistema calcula métricas importantes como:

- Waiting Time
- Turnaround Time
- Response Time
- Throughput
- Context Switches