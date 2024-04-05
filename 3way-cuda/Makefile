CC = nvcc
CFLAGS = -O3 -arch=sm_35 # Might need to adjust the architecture flag as per GPU

TARGET = cuda_program  # Name of the output executable

# Source files
SOURCES = cuda.cu

# Default build target
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o
