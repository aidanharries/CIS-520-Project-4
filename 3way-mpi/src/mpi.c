#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LINE_LENGTH 3000 // Max length of a single line

// Structure to hold memory usage information
typedef struct {
    uint32_t virtual_memory; // Virtual memory used by the process
    uint32_t physical_memory; // Physical memory used by the process
} process_memory_t;

/*
 * find_max 
 * Finds maximum ASCII value in lines
 * @param start_line Starting index of lines for this process
 * @param end_line Ending index of lines for this process
 * @param lines Pointer to array of lines
 * @param local_max_values Pointer to local max values array
 */
void find_max(int start_line, int end_line, char** lines, int* local_max_values) 
{
    for (int i = start_line; i < end_line; i++) {
        char *line = lines[i];
        int maxVal = 0;
        for (int j = 0; line[j] != '\0'; j++) {
            if (line[j] > maxVal) {
                maxVal = line[j];
            }
        }
        local_max_values[i - start_line] = maxVal;
    }
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

/*
 * main 
 * Entry point of the program
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status
 */
int main(int argc, char *argv[]) 
{
    int rank, num_procs;
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    // Get the rank of the process in the global communicator
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Get the number of processes in the global communicator
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Ensure the correct number of command-line arguments are passed
    if (argc < 3) {
        if (rank == 0) {
            printf("Usage: %s <filename> <max_lines>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char *filename = argv[1]; // Input filename from command line
    int max_lines = atoi(argv[2]); // Maximum number of lines to read from command line

    FILE *file = NULL;
    int total_lines = 0;
    char **lines = NULL;
    int *max_values = NULL;

    struct timeval start_time, end_time; // Variables for timing
    struct rusage usage_start, usage_end; // Variables for resource usage

    if (rank == 0) { // Code to execute by the root process only
        file = fopen(filename, "r"); // Open the file for reading
        if (!file) {
            fprintf(stderr, "ERROR: Could not open input file.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Allocate memory for line storage
        lines = (char **)malloc(max_lines * sizeof(char *));
        if (!lines) {
            fprintf(stderr, "Memory allocation failed for lines.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        char buffer[MAX_LINE_LENGTH];
        total_lines = 0;

        // Read lines from the file into the lines array
        while (fgets(buffer, MAX_LINE_LENGTH, file) && total_lines < max_lines) {
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
            lines[total_lines] = strdup(buffer); // Store the line
            total_lines++;
        }
        fclose(file); // Close the file

        // Allocate memory for max_values
        max_values = (int *)malloc(total_lines * sizeof(int));
        if (!max_values) {
            fprintf(stderr, "Memory allocation failed for max_values.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Broadcast the total number of lines to all processes
        MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // Broadcast each line to all processes
        for (int i = 0; i < total_lines; i++) {
            MPI_Bcast(lines[i], MAX_LINE_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        // Start timing and resource usage tracking
        gettimeofday(&start_time, NULL);
        getrusage(RUSAGE_SELF, &usage_start);
    } else {
        // Non-root processes receive the broadcast of total line count
        MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // Allocate memory for lines and receive the broadcasts
        lines = (char **)malloc(total_lines * sizeof(char *));
        if (!lines) {
            fprintf(stderr, "Memory allocation failed for lines.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < total_lines; i++) {
            lines[i] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
            if (!lines[i]) {
                fprintf(stderr, "Memory allocation failed for lines[%d].\n", i);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            MPI_Bcast(lines[i], MAX_LINE_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    }

    // Calculate which lines each process will handle
    int lines_per_proc = total_lines / num_procs;
    int remainder = total_lines % num_procs;
    int start_line = rank * lines_per_proc + (rank < remainder ? rank : remainder);
    int end_line = start_line + lines_per_proc + (rank < remainder ? 1 : 0);
    if (start_line >= total_lines) {
        start_line = end_line = total_lines; // Ensure no work for excess processes
    }

    // Local array for storing max values found by this process
    int *local_max_values = (int *)malloc((end_line - start_line) * sizeof(int));
    if (!local_max_values) {
        fprintf(stderr, "Memory allocation failed for local_max_values.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int i = 0; i < (end_line - start_line); i++) {
        local_max_values[i] = 0; // Initialize to 0
    }
    find_max(start_line, end_line, lines, local_max_values);

    int *recvcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        recvcounts = malloc(num_procs * sizeof(int));
        displs = malloc(num_procs * sizeof(int));
        int displacement = 0;
        for (int i = 0; i < num_procs; i++) {
            recvcounts[i] = (i < remainder) ? lines_per_proc + 1 : lines_per_proc;
            displs[i] = displacement;
            displacement += recvcounts[i];
        }
    }

    // Gather all max values found by all processes at the root process
    MPI_Gatherv(local_max_values, end_line - start_line, MPI_INT, max_values, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        free(recvcounts);
        free(displs);
    }

    if (rank == 0) {
        // Stop timing and resource usage tracking
        gettimeofday(&end_time, NULL);
        getrusage(RUSAGE_SELF, &usage_end);

        // Calculate and print performance metrics
        long seconds = end_time.tv_sec - start_time.tv_sec;
        long micros = (seconds * 1000000 + end_time.tv_usec) - start_time.tv_usec;

        long user_seconds = usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec;
        long user_microseconds = usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec;
        long system_seconds = usage_end.ru_stime.tv_sec - usage_start.ru_stime.tv_sec;
        long system_microseconds = usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec;

        process_memory_t myMem;
        get_process_memory(&myMem);

        // Print results
        for (int i = 0; i < total_lines; i++) {
            printf("%d: %d\n", i, max_values[i]);
        }

        // Output performance metrics and max values
        printf("\n");
        printf("Total runtime: %ld microseconds\n", micros);
        printf("User CPU time used: %ld seconds, %ld microseconds\n", user_seconds, user_microseconds);
        printf("System CPU time used: %ld seconds, %ld microseconds\n", system_seconds, system_microseconds);
        printf("Virtual memory used: %u KB\n", myMem.virtual_memory);
        printf("Physical memory used: %u KB\n", myMem.physical_memory);
        printf("\n");

    }

    // Free allocated memory
    free(local_max_values);
    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    if (rank == 0) {
        free(max_values);
    }

    // Clean up MPI environment
    MPI_Finalize();
    return 0;
}
