# === Makefile ===

# Compiler and flags
CC := gcc
CFLAGS := -std=c11 -O2

# Project name (output binary)
TARGET := main

# All .c files in the current directory
SRC := $(wildcard *.c)
# Object files (same names but with .o extension)
OBJ := $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Compile each .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remove compiled files
clean:
	rm -f $(OBJ) $(TARGET)

# Rebuild from scratch
rebuild: clean all

# Convenience target to run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
