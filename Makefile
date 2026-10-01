# variables del compilador y banderas
CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lm
TARGET = asciimov
SRC = asciimov.c

# ruta estandar para instalar binarios globales en linux
PREFIX = /usr/local/bin

# regla por defecto: compila el binario completo
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

# regla para limpiar los archivos compilados del disco
clean:
	rm -f $(TARGET)

# regla para compilar y ejecutar todo de una sola vez
run: all
	./$(TARGET)

# regla para instalar el programa de forma global en el sistema
install: all
	install -m 755 $(TARGET) $(PREFIX)

# regla para borrar el programa del sistema por completo
uninstall:
	rm -f $(PREFIX)/$(TARGET)
