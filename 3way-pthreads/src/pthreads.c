#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include "pthreads.h"

#define MAX_LINES 100
#define MAX_LINE_LENGTH 3000
#define MAX_THREADS 40

// Global variables
pthread_t threads[MAX_THREADS];
thread_data_t threadData[MAX_THREADS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR: Could not open input file.\n");
        exit(1);
    }

    char **lines = (char **)malloc(MAX_LINES * sizeof(char *));
    int *maxValues = (int *)malloc(MAX_LINES * sizeof(int));
    char buffer[MAX_LINE_LENGTH];
    int totalLines = 0;

    while (fgets(buffer, MAX_LINE_LENGTH, file) && totalLines < MAX_LINES) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        lines[totalLines] = strdup(buffer);
        totalLines++;
    }
    fclose(file);

    int linesPerThread = totalLines / MAX_THREADS;
    for (int i = 0; i < MAX_THREADS; i++) {
        threadData[i].id = i;
        threadData[i].start_line = i * linesPerThread;
        threadData[i].end_line = (i + 1) * linesPerThread;
        if (i == MAX_THREADS - 1) {
            threadData[i].end_line = totalLines; // Ensure the last thread covers the rest
        }
        threadData[i].lines = lines;
        threadData[i].max_values = maxValues;

        pthread_create(&threads[i], NULL, find_max, (void *)&threadData[i]);
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print results
    for (int i = 0; i < totalLines; i++) {
        printf("%d: %d\n", i, maxValues[i]);
    }

    // Cleanup
    for (int i = 0; i < totalLines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(maxValues);

    return 0;
}

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
