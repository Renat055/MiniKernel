@echo off
REM Script de compilacion para Windows

echo ========================================
echo   Compilando MiniKernel para Windows
echo ========================================

REM Crear directorio obj si no existe
if not exist obj mkdir obj

REM Compilar archivos fuente
echo Compilando archivos fuente...
gcc -Wall -Wextra -pthread -I./include -g -c src/main.c -o obj/main.o
gcc -Wall -Wextra -pthread -I./include -g -c src/queue.c -o obj/queue.o
gcc -Wall -Wextra -pthread -I./include -g -c src/cpu.c -o obj/cpu.o
gcc -Wall -Wextra -pthread -I./include -g -c src/metrics.c -o obj/metrics.o

REM Enlazar
echo Enlazando ejecutable...
gcc obj/main.o obj/queue.o obj/cpu.o obj/metrics.o -o minikernel.exe -pthread

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   Compilacion exitosa!
    echo   Ejecutable: minikernel.exe
    echo ========================================
    echo.
    echo Para ejecutar: minikernel.exe
) else (
    echo.
    echo ========================================
    echo   Error en la compilacion
    echo ========================================
)

pause
