# Compiles the pthreads max char finder program

# Directories
INCDIR = ../include
SRCDIR = ../src
OBJDIR = ./obj

# Compiler and flags
CC = gcc
CFLAGS = -I$(INCDIR) -Wall -Wextra -Wshadow -Werror -D_XOPEN_SOURCE=500 -pthread
LDFLAGS = -lpthread

# Create the obj directory if it doesn't exist
$(shell mkdir -p $(OBJDIR))

# Dependencies and Objects
_DEPS = pthreads.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = pthreads.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Target to compile the final executable
pthreads_program: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

# Clean target
.PHONY: clean
clean:
	rm -rf $(OBJDIR) *~ core $(INCDIR)/*~ pthreads_program
