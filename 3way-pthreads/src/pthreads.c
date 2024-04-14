#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "pthreads.h"

#define MAX_LINE_LENGTH 3000 // Max length of a single line
#define MAX_THREADS 40 // Absolute max number of threads

// Global variables
pthread_t threads[MAX_THREADS]; // Thread identifiers
thread_data_t threadData[MAX_THREADS]; // Thread data
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread synchronization
int active_threads = 0; // Counter for threads

/*
 * main 
 * Entry point of the program
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status
 */
int main(int argc, char *argv[]) 
{
    // Check for correct number of arguments
    if (argc < 4) {
        printf("Usage: %s <filename> <max_lines> <num_threads>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1]; // Input filename
    int max_lines = atoi(argv[2]); // Maximum lines to read
    int num_threads = atoi(argv[3]); // Number of threads to use

    if (num_threads > MAX_THREADS) num_threads = MAX_THREADS; // Cap the threads at MAX_THREADS

    FILE *file = fopen(filename, "r"); // Open file for reading
    if (!file) {
        fprintf(stderr, "ERROR: Could not open input file.\n");
        exit(1);
    }

    // Allocate memory for lines and maxValues
    char **lines = (char **)malloc(max_lines * sizeof(char *));
    int *maxValues = (int *)malloc(max_lines * sizeof(int));
    char buffer[MAX_LINE_LENGTH]; // Buffer to store line
    int totalLines = 0; // Total number of lines read

    // Read lines from file
    while (fgets(buffer, MAX_LINE_LENGTH, file) && totalLines < max_lines) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        lines[totalLines] = strdup(buffer); // Duplicate line
        totalLines++; // Increment totalLines
    }
    fclose(file); // Close file

    int linesPerThread = totalLines / num_threads; // Lines per thread

    // Start performance measurments
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    struct rusage usage_start;
    getrusage(RUSAGE_SELF, &usage_start);

    for (int i = 0; i < num_threads; i++) {
        // Set thread data
        threadData[i].id = i;
        threadData[i].start_line = i * linesPerThread;
        threadData[i].end_line = (i + 1) * linesPerThread;
        if (i == num_threads - 1) {
            threadData[i].end_line = totalLines; // Last thread takes remainder
        }
        threadData[i].lines = lines;
        threadData[i].max_values = maxValues;

        // Create thread
        if (pthread_create(&threads[i], NULL, find_max, (void *)&threadData[i]) == 0) {
            active_threads++; // Increment thread count
        }
    }

    // Join threads
    for (int i = 0; i < active_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print results
    for (int i = 0; i < totalLines; i++) {
        printf("%d: %d\n", i, maxValues[i]);
    }

    // End performance measurments
    gettimeofday(&end_time, NULL);
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long micros = (seconds * 1000000 + end_time.tv_usec) - start_time.tv_usec;

    struct rusage usage_end;
    getrusage(RUSAGE_SELF, &usage_end);

    process_memory_t myMem;
    get_process_memory(&myMem);

    // Calculate elapsed user time
    long user_seconds = usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec;
    long user_microseconds = usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec;

    // Normalize the time
    if (user_microseconds < 0) {
        user_seconds -= 1;
        user_microseconds += 1000000; 
    }

    // Calculate elapsed system time
    long system_seconds = usage_end.ru_stime.tv_sec - usage_start.ru_stime.tv_sec;
    long system_microseconds = usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec;

    // Normalize the time
    if (system_microseconds < 0) {
        system_seconds -= 1;
        system_microseconds += 1000000;
    }

    // Print Performance Metrics
    printf("\n");
    printf("Total runtime: %ld microseconds\n", micros); // The total execution time of the program
    printf("User CPU time used: %ld seconds, %ld microseconds\n", user_seconds, user_microseconds); // The amount of CPU time spent in user-mode code (outside the kernel) 
    printf("System CPU time used: %ld seconds, %ld microseconds\n", system_seconds, system_microseconds); // The amount of CPU time spent running system (kernel) code 
    printf("Virtual memory used: %u KB\n", myMem.virtual_memory); // The amount of virtual memory used by the process
    printf("Physical memory used: %u KB\n", myMem.physical_memory); // The amount of RAM used by the process
    printf("Total threads used: %d\n", active_threads); // Total number of threads used
    printf("\n");

    // Cleanup
    for (int i = 0; i < totalLines; i++) {
        free(lines[i]); // Free line memory
    }
    free(lines); // Free lines array
    free(maxValues); // Free maxValues array
    
    return 0;
}

/*
 * find_max 
 * Finds maximum value in line
 * @param args Pointer to thread_data_t
 */
void *find_max(void *args) 
{
    thread_data_t *data = (thread_data_t *)args;

    for (int i = data->start_line; i < data->end_line; i++) {
        char *line = data->lines[i];
        int maxVal = 0;
        for (int j = 0; line[j] != '\0'; j++) {
            if (line[j] > maxVal) {
                maxVal = line[j];
            }
        }
        data->max_values[i] = maxVal;
    }

    return NULL;
}

/*
 * get_process_memory
 * Finds the amount of virtual and physical memory used during the system process
 * @param processMem Pointer to the process_memory_t structure
 * 
 * Referenced: /homes/dan/625/checkmem.c
*/
void get_process_memory(process_memory_t* processMem) 
{
    FILE *file = fopen("/proc/self/status", "r");
    char line[128];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%u", &processMem->virtual_memory);
        }
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%u", &processMem->physical_memory);
        }
    }
    fclose(file);
}