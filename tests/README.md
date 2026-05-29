# Tests para MiniKernel

Este directorio está reservado para pruebas unitarias y de integración del sistema.

## Pruebas 

1. **Test de Cola de Procesos**
   - Verificar operaciones enqueue/dequeue
   - Probar sincronización con múltiples hilos
   - Validar condiciones de carrera

2. **Test de PCB**
   - Crear y destruir PCBs
   - Verificar cálculo de métricas

3. **Test de Scheduler**
   - Validar Round Robin con diferentes quantums
   - Probar con diferentes números de CPUs
   - Verificar fairness del algoritmo

4. **Test de Métricas**
   - Validar cálculos de tiempo de espera
   - Verificar throughput
   - Comprobar tiempo de respuesta

## Ejecución de Tests

```bash
# Compilar tests
make tests

# Ejecutar tests
./run_tests.sh
```
