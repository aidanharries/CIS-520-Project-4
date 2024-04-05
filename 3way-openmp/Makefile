CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Werror -fopenmp
LDFLAGS = -fopenmp

TARGET = openmp_program  # Name of the output executable

SOURCES = openmp.c  # Source files

# Default build target
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
