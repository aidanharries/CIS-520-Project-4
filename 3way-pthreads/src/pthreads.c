#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include "pthreads.h"

#define MAX_LINES 1000000 // Max number of lines to process
#define MAX_LINE_LENGTH 3000 // Max length of a single line
#define MAX_THREADS 40 // Max number of threads

// To do: Need to add performance evaluation code 

// Global variables
pthread_t threads[MAX_THREADS]; // Thread identifiers
thread_data_t threadData[MAX_THREADS]; // Thread data
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread synchronization

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
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1]; // Input filename
    FILE *file = fopen(filename, "r"); // Open file for reading
    if (!file) {
        fprintf(stderr, "ERROR: Could not open input file.\n");
        exit(1);
    }

    // Start performance measurments
    struct timespec* startTime = malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, startTime);

    // Allocate memory for lines and maxValues
    char **lines = (char **)malloc(MAX_LINES * sizeof(char *));
    int *maxValues = (int *)malloc(MAX_LINES * sizeof(int));
    char buffer[MAX_LINE_LENGTH]; // Buffer to store line
    int totalLines = 0; // Total number of lines read

    // Read lines from file
    while (fgets(buffer, MAX_LINE_LENGTH, file) && totalLines < MAX_LINES) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        lines[totalLines] = strdup(buffer); // Duplicate line
        totalLines++; // Increment totalLines
    }
    fclose(file); // Close file

    int linesPerThread = totalLines / MAX_THREADS; // Lines per thread
    for (int i = 0; i < MAX_THREADS; i++) {
        // Set thread data
        threadData[i].id = i;
        threadData[i].start_line = i * linesPerThread;
        threadData[i].end_line = (i + 1) * linesPerThread;
        if (i == MAX_THREADS - 1) {
            threadData[i].end_line = totalLines; // Last thread takes remainder
        }
        threadData[i].lines = lines;
        threadData[i].max_values = maxValues;

        // Create thread
        pthread_create(&threads[i], NULL, find_max, (void *)&threadData[i]);
    }

    // Join threads
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print results
    for (int i = 0; i < totalLines; i++) {
        printf("%d: %d\n", i, maxValues[i]);
    }

    // Cleanup
    for (int i = 0; i < totalLines; i++) {
        free(lines[i]); // Free line memory
    }
    free(lines); // Free lines array
    free(maxValues); // Free maxValues array

    // End performance measurments
    struct timespec* endTime = malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, endTime);

    // Print performance results
    if(endTime->tv_nsec - startTime->tv_nsec < 0){ // Prevent any overflow in the nanoseconds
        endTime->tv_sec--;
        endTime->tv_nsec += 1000000000; // take a second and move it into the nanoseconds for proper subtraction
    }
    printf("Performance: %lld seconds and %ld nano-seconds\n", 
        endTime->tv_sec - startTime->tv_sec,
        endTime->tv_nsec - startTime->tv_nsec);
    // printf("Start time: %lld seconds and %ld nano-seconds\n", startTime->tv_sec, startTime->tv_nsec);
    // printf("End time: %lld seconds and %ld nano-seconds\n", endTime->tv_sec, endTime->tv_nsec);
    
    return 0;
}

/*
 * find_max 
 * Finds maximum value in line
 * @param args Pointer to thread_data_t
 */
void *find_max(void *args) {
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
