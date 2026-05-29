# Makefile 

CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include -g
LDFLAGS = -pthread

# Directorios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = .

# Archivos
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/minikernel

# Regla principal
all: $(TARGET)

# Crear ejecutable
$(TARGET): $(OBJECTS)
	@echo "Enlazando $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Compilacion exitosa!"

# Compilar archivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Crear directorio de objetos
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos generados..."
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Limpieza completada."

# Ejecutar el programa
run: $(TARGET)
	@echo "Ejecutando MiniKernel..."
	./$(TARGET)

# Ayuda
help:
	@echo "Makefile para MiniKernel"
	@echo ""
	@echo "Uso:"
	@echo "  make          - Compilar el proyecto"
	@echo "  make clean    - Limpiar archivos generados"
	@echo "  make run      - Compilar y ejecutar"
	@echo "  make help     - Mostrar esta ayuda"

.PHONY: all clean run help
