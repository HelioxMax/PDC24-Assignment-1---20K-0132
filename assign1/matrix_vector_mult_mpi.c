#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void fill_matrix_vector(double *matrix, double *vector, int matrix_size, int vector_size) {
    // Fill matrix and vector with random values
    for (int i = 0; i < matrix_size * vector_size; i++) {
        matrix[i] = rand() % 10;
    }
    for (int i = 0; i < vector_size; i++) {
        vector[i] = rand() % 10;
    }
}

void print_matrix_vector(double *matrix, double *vector, int matrix_size, int vector_size) {
    printf("Matrix:\n");
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < vector_size; j++) {
            printf("%.2f\t", matrix[i * vector_size + j]);
        }
        printf("\n");
    }
    printf("\nVector:\n");
    for (int i = 0; i < vector_size; i++) {
        printf("%.2f\n", vector[i]);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int matrix_size, vector_size;
    double *matrix, *vector, *result, *local_result;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0)
            printf("Usage: %s <matrix_size> <vector_size>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    matrix_size = atoi(argv[1]);
    vector_size = atoi(argv[2]);

    if (matrix_size <= 0 || vector_size <= 0 || matrix_size != vector_size) {
        if (rank == 0)
            printf("Matrix and vector sizes must be positive integers and equal.\n");
        MPI_Finalize();
        return 1;
    }

    matrix = (double *)malloc(matrix_size * vector_size * sizeof(double));
    vector = (double *)malloc(vector_size * sizeof(double));
    result = (double *)malloc(matrix_size * sizeof(double));
    local_result = (double *)malloc(matrix_size / size * sizeof(double));

    if (rank == 0) {
        fill_matrix_vector(matrix, vector, matrix_size, vector_size);
    }

    MPI_Bcast(vector, vector_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double *local_matrix = (double *)malloc(matrix_size / size * vector_size * sizeof(double));

    MPI_Scatter(matrix, matrix_size / size * vector_size, MPI_DOUBLE, local_matrix, matrix_size / size * vector_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < matrix_size / size; i++) {
        local_result[i] = 0.0;
        for (int j = 0; j < vector_size; j++) {
            local_result[i] += local_matrix[i * vector_size + j] * vector[j];
        }
    }

    MPI_Gather(local_result, matrix_size / size, MPI_DOUBLE, result, matrix_size / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result:\n");
        for (int i = 0; i < matrix_size; i++) {
            printf("%.2f\n", result[i]);
        }
    }

    free(matrix);
    free(vector);
    free(result);
    free(local_matrix);
    free(local_result);

    MPI_Finalize();
    return 0;
}

