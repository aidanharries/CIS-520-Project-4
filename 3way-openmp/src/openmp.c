#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LINE_LENGTH 3000 // Max length of a single line


// Structure to hold memory usage information
typedef struct process_memory {
    uint32_t virtual_memory; // Virtual memory used by the process
    uint32_t physical_memory; // Physical memory used by the process
} process_memory_t;

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

int find_max(char* line) 
{
    int maxVal = line[0]; 
    for (int j = 0; line[j] != '\0'; j++) {
        if (line[j] > maxVal) {
            maxVal = line[j];
        }
    }
    return maxVal;
}

/*
 * main 
 * Entry point of the program
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status
 */
int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc < 4) {
        printf("Usage: %s <filename> <max_lines> <num_threads>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1]; // Input filename from command line
    int max_lines = atoi(argv[2]); // Maximum number of lines to read from command line
    int threads_num = atoi(argv[3]); // Number of threads to use

    FILE *file = NULL;
    int total_lines = 0;
    char **lines = NULL;
    int *max_values = NULL;

    struct timeval start_time, end_time; // Variables for timing
    struct rusage usage_start, usage_end; // Variables for resource usage

    file = fopen(filename, "r"); // Open the file for reading
    if (!file) {
        fprintf(stderr, "ERROR: Could not open input file.\n");
        exit(1);
    }

    // Allocate memory for lines
    lines = (char **)malloc(max_lines * sizeof(char *));
    if (!lines) {
        fprintf(stderr, "Memory allocation failed for lines.\n");
        exit(1);
    } 
    for (int i = 0; i < max_lines; i++) {
        lines[i] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
        if (!lines[i]) {
            fprintf(stderr, "Memory allocation failed for lines[%d].\n", i);
            exit(1);
        }
    }

    char *buffer = malloc(MAX_LINE_LENGTH);
    total_lines = 0;

    // Read lines from file
    while (fgets(buffer, MAX_LINE_LENGTH, file) && total_lines < max_lines) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        lines[total_lines] = strdup(buffer); // Duplicate line
        total_lines++; // Increment totalLines
    }
    fclose(file); // Close the file

    // Allocate memory for max_values
    max_values = (int *)malloc(total_lines * sizeof(int));
    if (!max_values) {
        fprintf(stderr, "Memory allocation failed for max_values.\n");
        exit(1);
    }

    // Start timing and resource usage tracking
    gettimeofday(&start_time, NULL);
    getrusage(RUSAGE_SELF, &usage_start);

    // Parrallelize Max Lines
    omp_set_num_threads(threads_num);
    #pragma omp parallel for shared(lines) 
    for (int i = 0; i < total_lines; i++){
        max_values[i] = find_max(lines[i]);
        // printf("Thread: %d\n", omp_get_thread_num());
    }


    // Stop timing and resource usage tracking
    gettimeofday(&end_time, NULL);
    getrusage(RUSAGE_SELF, &usage_end);

    // Calculate and print performance metrics
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long micros = end_time.tv_usec - start_time.tv_usec;

    // Normalize the microseconds
    if (micros < 0) {
        micros += 1000000;  // adjust by one second
        seconds -= 1;
    }

    long total_micros = seconds * 1000000 + micros;

    // Calculate user and system CPU time used
    long user_seconds = usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec;
    long user_microseconds = usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec;

    // Normalize user CPU time
    if (user_microseconds < 0) {
        user_microseconds += 1000000;
        user_seconds -= 1;
    }

    long system_seconds = usage_end.ru_stime.tv_sec - usage_start.ru_stime.tv_sec;
    long system_microseconds = usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec;

    // Normalize system CPU time
    if (system_microseconds < 0) {
        system_microseconds += 1000000;
        system_seconds -= 1;
    }

    process_memory_t myMem;
    get_process_memory(&myMem);

    // Print results
    for (int i = 0; i < total_lines; i++) {
        printf("%d: %d\n", i, max_values[i]);
    }

    // Output performance metrics
    printf("\n");
    printf("Total runtime: %ld microseconds\n", total_micros);
    printf("User CPU time used: %ld seconds, %ld microseconds\n", user_seconds, user_microseconds);
    printf("System CPU time used: %ld seconds, %ld microseconds\n", system_seconds, system_microseconds);
    printf("Virtual memory used: %u KB\n", myMem.virtual_memory);
    printf("Physical memory used: %u KB\n", myMem.physical_memory);
    printf("\n");

    // Free allocated memory
    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(max_values);

    return 0;
}


