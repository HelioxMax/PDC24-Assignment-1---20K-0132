#!/bin/bash

# Function to check the exit status of the previous command
check_exit_status() {
    if [ $? -ne 0 ]; then
        echo "Error: Previous command failed. Exiting."
        exit 1
    fi
}

# Compile all C programs
gcc -o matrix_vector_mult_seq matrix_vector_mult_seq.c -lm
check_exit_status

gcc -fopenmp -o matrix_vector_mult_openmp matrix_vector_mult_openmp.c -lm
check_exit_status

mpicc -o matrix_vector_mult_mpi matrix_vector_mult_mpi.c -lm
check_exit_status

gcc -fopenmp -o matrix_vector_mult_openmp_tiled matrix_vector_mult_openmp_tiled.c -lm
check_exit_status

mpicc -o matrix_vector_mult_mpi_tiled matrix_vector_mult_mpi_tiled.c -lm
check_exit_status

# Define input sizes
input_sizes=(64 128 256 512 1024 2048 4096 8192 16384 32768)

# Benchmark each program for different input sizes
for size in ${input_sizes[@]}; do
    # Run sequential program
    avg_time_seq=0
    for ((i=0; i<10; i++)); do
        time_seq=$(./matrix_vector_mult_seq $size $size | grep real | awk '{print $2}')
        avg_time_seq=$(awk "BEGIN {print \"$avg_time_seq\" + \"$time_seq\"}")
    done
    avg_time_seq=$(awk "BEGIN {print \"$avg_time_seq\" / 10}")
    echo "seq, $size, $avg_time_seq"

    # Run OpenMP program
    avg_time_openmp=0
    for ((i=0; i<10; i++)); do
        time_openmp=$(./matrix_vector_mult_openmp $size $size | grep real | awk '{print $2}')
        avg_time_openmp=$(awk "BEGIN {print \"$avg_time_openmp\" + \"$time_openmp\"}")
    done
    avg_time_openmp=$(awk "BEGIN {print \"$avg_time_openmp\" / 10}")
    echo "openmp, $size, $avg_time_openmp"

    # Run MPI program
    avg_time_mpi=0
    for ((i=0; i<10; i++)); do
        time_mpi=$(mpiexec -n 4 ./matrix_vector_mult_mpi $size $size | grep real | awk '{print $2}')
        avg_time_mpi=$(awk "BEGIN {print \"$avg_time_mpi\" + \"$time_mpi\"}")
    done
    avg_time_mpi=$(awk "BEGIN {print \"$avg_time_mpi\" / 10}")
    echo "mpi, $size, $avg_time_mpi"

    # Run OpenMP tiled program
    avg_time_openmp_tiled=0
    for ((i=0; i<10; i++)); do
        time_openmp_tiled=$(./matrix_vector_mult_openmp_tiled $size $size | grep real | awk '{print $2}')
        avg_time_openmp_tiled=$(awk "BEGIN {print \"$avg_time_openmp_tiled\" + \"$time_openmp_tiled\"}")
    done
    avg_time_openmp_tiled=$(awk "BEGIN {print \"$avg_time_openmp_tiled\" / 10}")
    echo "openmp_tiled, $size, $avg_time_openmp_tiled"

    # Run MPI tiled program
    avg_time_mpi_tiled=0
    for ((i=0; i<10; i++)); do
        time_mpi_tiled=$(mpiexec -n 4 ./matrix_vector_mult_mpi_tiled $size $size | grep real | awk '{print $2}')
        avg_time_mpi_tiled=$(awk "BEGIN {print \"$avg_time_mpi_tiled\" + \"$time_mpi_tiled\"}")
    done
    avg_time_mpi_tiled=$(awk "BEGIN {print \"$avg_time_mpi_tiled\" / 10}")
    echo "mpi_tiled, $size, $avg_time_mpi_tiled"
done

