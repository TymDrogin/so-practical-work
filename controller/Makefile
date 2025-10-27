# === Makefile ===

# Compiler and flags
CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra -Iinclude  # include headers from include/

# Source and header files
SRC := $(wildcard src/*.c)
HEADERS := $(wildcard include/*.h)

# Object files (in build folder)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

# Output binary (inside build folder)
TARGET := build/controller

# Default target: clean then build
all: clean $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Compile each .c file into a .o file (create build dir if missing)
build/%.o: src/%.c $(HEADERS)
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
	cd build && ./controller

.PHONY: all clean rebuild run
