# Compiles the OpenMP max char finder program

# Directories
SRCDIR = ../src
OBJDIR = ./obj

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Werror -fopenmp

# Create the obj directory if it doesn't exist
$(shell mkdir -p $(OBJDIR))

# Objects
_OBJ = openmp.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Target to compile the final executable
openmp_program: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Clean target
.PHONY: clean
clean:
	rm -rf $(OBJDIR) *~ core openmp_program
