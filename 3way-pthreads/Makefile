CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Werror -D_XOPEN_SOURCE=500
LDFLAGS = -lpthread

TARGET = pthreads_program  # Name of the output executable

SOURCES = pthreads.c  # Source files

# Default build target
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
