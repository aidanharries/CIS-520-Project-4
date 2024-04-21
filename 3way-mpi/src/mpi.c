#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LINE_LENGTH 3000 // Max length of a single line

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
 * Retrieves process memory information
 * @param virtual_memory Pointer to the virtual memory used
 * @param physical_memory Pointer to the physical memory used
 *
 * Referenced: /homes/dan/625/checkmem.c
 */
void get_process_memory(unsigned int* virtual_memory, unsigned int* physical_memory) 
{
    FILE *file = fopen("/proc/self/status", "r");
    char line[128];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%u", virtual_memory);
        }
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%u", physical_memory);
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
    MPI_Init(&argc, &argv); // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // Get the total number of processes

    if (argc < 3) {
        if (rank == 0) {
            printf("Usage: %s <filename> <max_lines>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char *filename = argv[1]; // Input filename
    int max_lines = atoi(argv[2]); // Maximum lines to read

    FILE *file = NULL;
    if (rank == 0) {
        file = fopen(filename, "r"); // Open file for reading
        if (!file) {
            fprintf(stderr, "ERROR: Could not open input file.\n");
            MPI_Finalize();
            return 1;
        }
    }

    // Distribute the lines among processes
    int total_lines = 0;
    char **lines = NULL;
    int *max_values = NULL;

    if (rank == 0) {
        lines = (char **)malloc(max_lines * sizeof(char *));
        char buffer[MAX_LINE_LENGTH]; // Buffer to store line
        total_lines = 0;

        // Read lines from file
        while (fgets(buffer, MAX_LINE_LENGTH, file) && total_lines < max_lines) {
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline
            lines[total_lines] = strdup(buffer); // Duplicate line
            total_lines++; // Increment total_lines
        }

        fclose(file); // Close file

        // Scatter line data to all processes
        MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcast total_lines
        for (int i = 0; i < total_lines; i++) {
            MPI_Bcast(lines[i], MAX_LINE_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD); // Broadcast each line
        }
    } else {
        MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD); // Receive total_lines
        lines = (char **)malloc(total_lines * sizeof(char *));
        for (int i = 0; i < total_lines; i++) {
            lines[i] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
            MPI_Bcast(lines[i], MAX_LINE_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD); // Receive each line
        }
    }

    int lines_per_proc = total_lines / num_procs; // Lines per process
    int start_line = rank * lines_per_proc; // Starting line for this process
    int end_line = (rank + 1) * lines_per_proc; // Ending line for this process
    if (rank == num_procs - 1) {
        end_line = total_lines; // Last process takes remainder
    }

    if (rank == 0) {
        max_values = (int *)malloc(total_lines * sizeof(int)); // Allocate max values
    }

    int *local_max_values = (int *)malloc((end_line - start_line) * sizeof(int)); // Local max values

    // Find max values for this process's portion of lines
    find_max(start_line, end_line, lines, local_max_values);

    // Gather the max values from all processes
    MPI_Gather(local_max_values, end_line - start_line, MPI_INT, max_values, end_line - start_line, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Print results
        for (int i = 0; i < total_lines; i++) {
            printf("%d: %d\n", i, max_values[i]);
        }

        // Cleanup
        free(max_values);
    }

    // Free memory
    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(local_max_values);

    MPI_Finalize(); // Finalize MPI
    return 0;
}
