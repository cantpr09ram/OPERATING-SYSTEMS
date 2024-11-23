#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>

#define MAX_ROWS 1000
#define NUM_COLUMNS 4
#define MAX_OUTPUT_SIZE 4096
#define MAX_NUMBERS (MAX_ROWS * NUM_COLUMNS)

int data[MAX_ROWS][NUM_COLUMNS];  // Raw input data
int sorted_data[MAX_NUMBERS];    // Merged sorted data
int row_count = 0;               // Number of rows in the input file
int column_sums[NUM_COLUMNS] = {0}; // Column sums for each thread

char thread_outputs[NUM_COLUMNS][MAX_OUTPUT_SIZE]; // Buffer to store thread outputs
pthread_mutex_t output_mutex;          // Mutex for thread outputs
pthread_mutex_t file_mutex;            // Mutex for file access

typedef struct {
    int column[MAX_ROWS];
    int size;
    int thread_no;
} ThreadData;

// Function to read the input file
void read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d,%d,%d", &data[row_count][0], &data[row_count][1],
               &data[row_count][2], &data[row_count][3]);
        row_count++;
    }
    fclose(file);
}

// Bubble sort function
void bubble_sort(int *arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Thread function to sort a column and calculate its sum
void *sort_and_sum(void *arg) {
    ThreadData *thread_data = (ThreadData *)arg;

    //printf("Thread %d is processing %d items.\n", thread_data->thread_no, thread_data->size);

    int sum = 0;

    // Sort the column
    bubble_sort(thread_data->column, thread_data->size);

    // Calculate the sum of the column
    for (int i = 0; i < thread_data->size; i++) {
        sum += thread_data->column[i];
    }
    // Store thread output
    column_sums[thread_data->thread_no - 1] = sum;

    // try lock output_mutex to write to thread_outputs
    int ret = pthread_mutex_lock(&output_mutex);
    if (ret != 0) {
        fprintf(stderr, "Error locking output_mutex: %s\n", strerror(ret));
        pthread_exit(NULL);
    }

    // Initialize output string
    snprintf(thread_outputs[thread_data->thread_no - 1], MAX_OUTPUT_SIZE, "Thread %d: ", thread_data->thread_no);

    // Append sorted data
    for (int i = 0; i < thread_data->size; i++) {
        size_t len = strlen(thread_outputs[thread_data->thread_no - 1]);
        snprintf(thread_outputs[thread_data->thread_no - 1] + len, MAX_OUTPUT_SIZE - len, "%d,", thread_data->column[i]);
    }
    // Remove the last comma
    size_t len = strlen(thread_outputs[thread_data->thread_no - 1]);
    if (len > 0 && thread_outputs[thread_data->thread_no - 1][len - 1] == ',') {
        thread_outputs[thread_data->thread_no - 1][len - 1] = '\0';
    }

    // Append sum information
    len = strlen(thread_outputs[thread_data->thread_no - 1]);
    snprintf(thread_outputs[thread_data->thread_no - 1] + len, MAX_OUTPUT_SIZE - len, "\nsum %d: %d\n", thread_data->thread_no, sum);

    // Unlock output_mutex
    ret = pthread_mutex_unlock(&output_mutex);
    if (ret != 0) {
        fprintf(stderr, "Error unlocking output_mutex: %s\n", strerror(ret));
        pthread_exit(NULL);
    }

    pthread_exit(NULL);
}

// Function to merge sorted columns into one array
void *merge_sorted_data(void *arg) {
    int idx[NUM_COLUMNS] = {0}; // Indexes for each column
    int current = 0;

    // Merge sorted columns into a single array
    while (current < row_count * NUM_COLUMNS) {
        int min_value = INT_MAX;
        int col_index = -1;

        for (int i = 0; i < NUM_COLUMNS; i++) {
            if (idx[i] < row_count && data[idx[i]][i] < min_value) {
                min_value = data[idx[i]][i];
                col_index = i;
            }
        }
        sorted_data[current++] = min_value;
        idx[col_index]++;
    }

    // Write merged sorted data to file
    pthread_mutex_lock(&file_mutex);
    FILE *output_file = fopen("output.txt", "a");
    if (!output_file) {
        perror("Error writing to file");
        pthread_mutex_unlock(&file_mutex);
        pthread_exit(NULL);
    }

    fprintf(output_file, "=================================================\n");
    for (int i = 0; i < current; i++) {
        fprintf(output_file, "%d", sorted_data[i]);
        if (i < current - 1) {
            fprintf(output_file, ",");
        }
    }
    fprintf(output_file, "\n");
    fclose(output_file);
    pthread_mutex_unlock(&file_mutex);

    pthread_exit(NULL);
}

// Main function
int main() {
    // Initialize mutexes
    pthread_mutex_init(&output_mutex, NULL);
    pthread_mutex_init(&file_mutex, NULL);

    // Input file processing
    char input_file[100];
    FILE *file;

    while (1) {
        printf("Enter the input file name: ");
        scanf("%s", input_file);

        // Attempt to open the file
        file = fopen(input_file, "r");
        if (file) {
            // File exists, proceed
            fclose(file);
            break;
        } else {
            // File doesn't exist, print error and prompt again
            perror("Error opening file");
            printf("The file does not exist. Please try again.\n");
        }
    }
    read_file(input_file);

    // Create threads for sorting and summing
    pthread_t threads[NUM_COLUMNS + 1];
    ThreadData thread_data[NUM_COLUMNS];

    for (int i = 0; i < NUM_COLUMNS; i++) {
        thread_data[i].size = row_count;
        thread_data[i].thread_no = i + 1;
        for (int j = 0; j < row_count; j++) {
            thread_data[i].column[j] = data[j][i];
        }
        pthread_create(&threads[i], NULL, sort_and_sum, &thread_data[i]);
    }

    // Wait for all sorting threads to complete
    for (int i = 0; i < NUM_COLUMNS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Write thread outputs to file
    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < NUM_COLUMNS; i++) {
        fprintf(output_file, "%s", thread_outputs[i]);
    }
    fclose(output_file);

    // Create and run merge thread
    pthread_create(&threads[NUM_COLUMNS], NULL, merge_sorted_data, NULL);
    pthread_join(threads[NUM_COLUMNS], NULL);

    // Destroy mutexes
    pthread_mutex_destroy(&output_mutex);
    pthread_mutex_destroy(&file_mutex);

    printf("Processing complete. Results are in output.txt.\n");
    return 0;
}
