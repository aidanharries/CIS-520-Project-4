# Compiles the MPI max char finder program

# Directories
SRCDIR = ../src
OBJDIR = ./obj

# Compiler
CC = mpicc
CFLAGS = -Wall -Wextra -Wshadow -Werror

# Create the obj directory if it doesn't exist
$(shell mkdir -p $(OBJDIR))

# Dependencies and Objects
_OBJ = mpi.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Target to compile the final executable
mpi_program: $(OBJ)
	$(CC) -o $@ $^

# Clean target
.PHONY: clean
clean:
	rm -rf $(OBJDIR) *~ core mpi_program
