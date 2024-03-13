#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TILE_SIZE 16

// Function to allocate memory for a matrix
double **allocate_matrix(int rows, int cols) {
    double **matrix = (double **)malloc(rows * sizeof(double *));
    if (matrix == NULL) {
        fprintf(stderr, "Failed to allocate memory for matrix\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double *)malloc(cols * sizeof(double));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for matrix row %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}

// Function to free memory allocated to a matrix
void free_matrix(double **matrix, int rows) {
    for (int i = 0; i < rows; i++)
        free(matrix[i]);
    free(matrix);
}

// Function to perform matrix-vector multiplication with tiling
void matrix_vector_mult_tiled(double **matrix, double *vector, double *result, int rows, int cols, int rank, int size) {
    for (int i = rank * TILE_SIZE; i < ((rank + 1) * TILE_SIZE) && i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <matrix_size> <vector_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int matrix_size = atoi(argv[1]);
    int vector_size = atoi(argv[2]);

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double **matrix;
    double *vector = NULL;
    double *result = NULL;

    if (rank == 0) {
        matrix = allocate_matrix(matrix_size, vector_size);
        vector = (double *)malloc(vector_size * sizeof(double));
        result = (double *)calloc(matrix_size, sizeof(double));

        // Fill matrix and vector with random values
        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < vector_size; j++) {
                matrix[i][j] = (double)rand() / RAND_MAX;
            }
        }
        for (int i = 0; i < vector_size; i++) {
            vector[i] = (double)rand() / RAND_MAX;
        }
    }

    // Broadcast vector to all processes
    MPI_Bcast(vector, vector_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Allocate memory for local_matrix as a contiguous block
    int local_rows = (matrix_size + size - 1) / size;
    double *local_matrix_data = (double *)malloc(local_rows * vector_size * sizeof(double));
    double **local_matrix = (double **)malloc(local_rows * sizeof(double *));
    if (local_matrix_data == NULL || local_matrix == NULL) {
        fprintf(stderr, "Failed to allocate memory for local matrix\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < local_rows; i++) {
        local_matrix[i] = &local_matrix_data[i * vector_size];
    }

    // Scatter matrix rows to different processes
    MPI_Scatter(matrix[0], local_rows * vector_size, MPI_DOUBLE, local_matrix[0], local_rows * vector_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Allocate memory for local result
    double *local_result = (double *)calloc(local_rows, sizeof(double));

    // Perform matrix-vector multiplication with tiling
    matrix_vector_mult_tiled(local_matrix, vector, local_result, local_rows, vector_size, rank, size);

    // Gather results from all processes
    MPI_Gather(local_result, local_rows, MPI_DOUBLE, result, local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Print the final result
        for (int i = 0; i < matrix_size; i++) {
            printf("%f ", result[i]);
        }
        printf("\n");
    }

    // Free allocated memory
    if (rank == 0) {
        free_matrix(matrix, matrix_size);
        free(vector);
        free(result);
    }
    free(local_matrix_data);
    free(local_matrix);
    free(local_result);

    MPI_Finalize();

    return 0;
}

