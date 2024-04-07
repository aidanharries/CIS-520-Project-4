#ifndef PTHREADS_H__
#define PTHREADS_H__

#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structure to hold thread parameters
typedef struct thread_data {
    int id; // Identifier for each thread
    int start_line; // Starting line index for this thread to process
    int end_line; // Ending line index for this thread to process
    char** lines; // Pointer to the array of lines (shared among threads)
    int* max_values; // Array to store maximum ASCII values found by each thread
} thread_data_t;

// Structure to hold memory usage information
typedef struct process_memory {
    uint32_t virtual_memory; // Virtual memory used by the process
    uint32_t physical_memory; // Physical memory used by the process
} process_memory_t;

// Function prototype for the thread function to find maximum ASCII values
void* find_max(void* args);

// Function prototype to retrieve process memory usage
void get_process_memory(process_memory_t* process_memory);

#ifdef __cplusplus
}
#endif

#endif
