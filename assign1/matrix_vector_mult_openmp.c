#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to allocate memory for a matrix
double **allocate_matrix(int rows, int cols) {
    double **matrix = (double **)malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++)
        matrix[i] = (double *)malloc(cols * sizeof(double));
    return matrix;
}

// Function to free memory allocated to a matrix
void free_matrix(double **matrix, int rows) {
    for (int i = 0; i < rows; i++)
        free(matrix[i]);
    free(matrix);
}

// Function to perform matrix-vector multiplication
void matrix_vector_mult(double **matrix, double *vector, double *result, int rows, int cols) {
    #pragma omp parallel for
    for (int i = 0; i < rows; i++) {
        result[i] = 0.0;
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <matrix_size> <vector_size>\n", argv[0]);
        return 1;
    }

    int matrix_size = atoi(argv[1]);
    int vector_size = atoi(argv[2]);

    // Allocate memory for matrix and vector
    double **matrix = allocate_matrix(matrix_size, vector_size);
    double *vector = (double *)malloc(vector_size * sizeof(double));
    double *result = (double *)malloc(matrix_size * sizeof(double));

    // Fill matrix and vector with random values
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < vector_size; j++) {
            matrix[i][j] = (double)rand() / RAND_MAX;
        }
    }
    for (int i = 0; i < vector_size; i++) {
        vector[i] = (double)rand() / RAND_MAX;
    }

    // Perform matrix-vector multiplication
    matrix_vector_mult(matrix, vector, result, matrix_size, vector_size);

    // Free allocated memory
    free_matrix(matrix, matrix_size);
    free(vector);
    free(result);

    return 0;
}

