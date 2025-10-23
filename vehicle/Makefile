# === Makefile ===

# Compiler and flags
CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra

# Source and header files
SRC := $(wildcard *.c)
HEADERS := $(wildcard *.h)

# Object files (in build folder)
OBJ := $(patsubst %.c,build/%.o,$(SRC))

# Output binary (inside build folder)
TARGET := build/vehicle

# Default target: clean then build
all: clean $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Compile each .c file into a .o file (create build dir if missing)
build/%.o: %.c $(HEADERS)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Remove compiled files
clean:
	rm -rf build

# Rebuild from scratch (alias)
rebuild: clean all

# Convenience target to run the program
run: $(TARGET)
	# Run from parent directory so relative paths like "../file.txt" work
	cd build && ./client

.PHONY: all clean rebuild run
