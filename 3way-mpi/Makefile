CC = mpicc
CFLAGS = -Wall -Wextra -Wshadow -Werror

TARGET = mpi_program  # Name of the output executable

SOURCES = mpi.c  # Source files

# Default build target
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o
