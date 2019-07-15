/*
 * MIT License
 *
 * Copyright (c) 2019 Philip Kovacs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifdef HAVE_ATTRIBUTE_CLEANUP
#define AUTO_PTR(fn) __attribute__((cleanup(fn)))
#else
#define AUTO_PTR(fn)
#endif

void free_buffer(int **A);
void initialize(int argc, char *argv[], int *N, int **A, int **B, int **C);
void matrix_mult(int N, int *A, int *B, int *C);
void matrix_read(FILE *fp, int *N, int **A, int **B);
void matrix_print(const char *desc, int N, int *A);

/*
 * Print program usage.
 */
void usage()
{
    fprintf(stderr, "usage: cannon <options>\n"
                    "  Options are:\n"
                    "    --help|-h:        print this help\n"
                    "    --matrix|-m:      matrix input file\n"
    );
}

/*
 * Read a file of two square (N x N) matrices and multiply them in parallel
 * using Cannon's generalized algorithm. The number of processes assigned
 * must be in the range [1 <= np <= N*N] where np is a perfect square and
 * N/sqrt(np) is an integral number.
 *
 * If 1 process is indicated, sequential multiplication is used which can be
 * useful for reference to the parallel algorithm.
 *
 * If N*N is indicated, each cell of the resultant matrix is assigned a single
 * process.
 *
 * If np is indicated, a submatrix (block) of size N/sqrt(np) x N/sqrt(np)
 * is assigned to each process.
 *
 * Example:
 *
 * Two 6x6 matrices may be multiplied sequentially with np = 1 or in parallel
 * with np = 4 (4 blocks of 3x3); np = 9 (9 blocks of 2x2); or np = 36
 * (1 cell per process).
 */
int main(int argc, char *argv[])
{
    // Rank 0 matrices
    AUTO_PTR(free_buffer) int *A = NULL;
    AUTO_PTR(free_buffer) int *B = NULL;
    AUTO_PTR(free_buffer) int *C = NULL;

    // Local submatrices
    AUTO_PTR(free_buffer) int *local_A = NULL;
    AUTO_PTR(free_buffer) int *local_B = NULL;
    AUTO_PTR(free_buffer) int *local_C = NULL;

    int N = 0;
    int i;
    int rank, procs;
    int left, right, down, up;
    int coords[2];
    const int periods[2] = { 1, 1 };
    const int starts[2] = { 0, 0 };
    const int reorder = 1;
    MPI_Comm cart_comm;
    MPI_Datatype block_t, resized_block_t;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    if (rank == 0) {
        initialize(argc, argv, &N, &A, &B, &C);
        if (procs == 1) {
            // Use sequential multiplication if just 1 proc
            printf("Using sequential multiplication on 1 process.\n");
            matrix_print("Matrix A", N, A);
            matrix_print("Matrix B", N, B);
            matrix_mult(N, A, B, C);
            matrix_print("Matrix C", N, C);
        }
    }

    if (procs == 1) {
        MPI_Finalize();
        return 0;
    }

    // Number of processes must be a perfect square
    const double fprocs_sqrt = sqrt(procs);
    const int procs_sqrt = fprocs_sqrt;
    if (procs_sqrt != fprocs_sqrt) {
        if (rank == 0) {
            fprintf(stderr, "Number of processes (%d) is not a perfect square\n", procs);
        }
        MPI_Finalize();
        return 0;
    }

    // Broadcast N, the matrix dimension, to all processes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Matrices must be partitioned into equal sized blocks
    if (N % procs_sqrt != 0) {
        if (rank == 0) {
            fprintf(stderr, "Cannot partition %dx%d matrices on %d processes. "
                    "Ensure N/SQRT(processes) is an integral number.\n", N, N, procs);
        }
        MPI_Finalize();
        return 0;
    }

    // Runtime knowledge
    const int N_sub = N / procs_sqrt;
    const int N_sub_squared = N_sub * N_sub;
    const int array_sizes[2] = { N, N };
    const int block_sizes[2] = { N_sub, N_sub };
    const int cart_dims[2] = { procs_sqrt, procs_sqrt };
    AUTO_PTR(free_buffer) int *block_counts = calloc(procs, sizeof(int));
    AUTO_PTR(free_buffer) int *block_displs = calloc(procs, sizeof(int));

    // One block per process
    for (i = 0; i < procs; ++i) {
        block_counts[i] = 1;
    }

    // The starting extents of each block
    for (i = 0; i < procs; ++i) {
        int row = i / procs_sqrt;
        int col = i % procs_sqrt;
        block_displs[i] = row * N_sub * procs_sqrt + col;
    }

    // Create subarray type to scatter blocks with one message
    MPI_Type_create_subarray(2, array_sizes, block_sizes, starts, MPI_ORDER_C,
                             MPI_INT, &block_t);
    MPI_Type_create_resized(block_t, 0, N_sub * sizeof(int), &resized_block_t);
    MPI_Type_commit(&resized_block_t);

    // Use a cartesian process topology
    MPI_Cart_create(MPI_COMM_WORLD, 2, cart_dims, periods, reorder, &cart_comm);
    MPI_Comm_rank(cart_comm, &rank);

    // Allocate local submatrices (blocks)
    local_A = calloc(N_sub_squared, sizeof(*local_A));
    local_B = calloc(N_sub_squared, sizeof(*local_B));
    local_C = calloc(N_sub_squared, sizeof(*local_C));

    // Rank 0 scatters blocks of size N_sub x N_sub to all processes
    MPI_Scatterv(A, block_counts, block_displs, resized_block_t,
                 local_A, N_sub_squared, MPI_INT, 0, cart_comm);
    MPI_Scatterv(B, block_counts, block_displs, resized_block_t,
                 local_B, N_sub_squared, MPI_INT, 0, cart_comm);

    // Use cartesian coordinates to guide Cannon's initial block shifts:
    // Row 0 shifts left 0 ranks, row 1 shifts left 1 rank, etc.
    // Col 0 shifts up 0 ranks, col 1 shifts up 1 rank, etc.
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    MPI_Cart_shift(cart_comm, 1, coords[0], &left, &right);
    MPI_Cart_shift(cart_comm, 0, coords[1], &up, &down);
    MPI_Sendrecv_replace(local_A, N_sub_squared, MPI_INT, left, 1, right, 1,
                         cart_comm, MPI_STATUS_IGNORE);
    MPI_Sendrecv_replace(local_B, N_sub_squared, MPI_INT, up, 1, down, 1,
                         cart_comm, MPI_STATUS_IGNORE);

    // Set left and up block shifts to 1 rank for the rest of the algorithm
    MPI_Cart_shift(cart_comm, 1, 1, &left, &right);
    MPI_Cart_shift(cart_comm, 0, 1, &up, &down);

    if (rank == 0) {
        printf("Partitioned the %dx%d matrices on %d processes of %dx%d each.\n",
               N, N, procs, N_sub, N_sub);
        matrix_print("Matrix A", N, A);
        matrix_print("Matrix B", N, B);
    }

    // Each process multiplies, accumulates and shifts its local data
    for (i = 0; i < procs_sqrt; ++i) {
        // Multiply and accumulate local block
        matrix_mult(N_sub, local_A, local_B, local_C);

        // Shift block local_A left by one rank and local_B up by one rank
        MPI_Sendrecv_replace(local_A, N_sub_squared, MPI_INT, left, 1,
                             right, 1, cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv_replace(local_B, N_sub_squared, MPI_INT, up, 1,
                             down, 1, cart_comm, MPI_STATUS_IGNORE);
    }

    // Rank 0 gathers the final C matrix from all process local_C blocks
    MPI_Gatherv(local_C, N_sub_squared, MPI_INT, C, block_counts,
                block_displs, resized_block_t, 0, cart_comm);

    if (rank == 0) {
        matrix_print("Matrix C", N, C);
    }

    MPI_Type_free(&resized_block_t);
    MPI_Finalize();

#ifndef HAVE_ATTRIBUTE_CLEANUP
    if (rank == 0) {
        free(A);
        free(B);
        free(C);
    }
    free(local_A);
    free(local_B);
    free(local_C);
    free(block_count);
    free(block_displs);
#endif

    return 0;
}


/*
 * Free a buffer.
 */
void free_buffer(int **A)
{
    free (*A);
}

/*
 * Initialize rank 0 only: parse options, read and set up matrices.
 */
void initialize(int argc, char *argv[], int *N, int **A, int **B, int **C)
{
    FILE *fp = NULL;
    char filename[256];
    int c, help = 0;

    memset(filename, '\0', sizeof(filename));

    while (1) {
        static struct option long_options[] = {
            {"help",       no_argument,       0, 'h' },
            {"matrix",     required_argument, 0, 'm' },
            {0, 0, 0, 0}
        };
        
        int option_index = 0;
        c = getopt_long(argc, argv, "hm:", long_options, &option_index);
               
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                help = 1;
                break;
            case 'm':
                strncpy(filename, optarg, sizeof(filename)-1);
                break;
            default:
                break;
        }
    }

    if (help) {
        usage();
        exit(0);
    }
    if (filename[0] == '\0') {
        usage();
        exit(2);
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr,"%s (%s)\n", strerror(errno), filename);
        exit(1);
    }
    matrix_read(fp, N, A, B);
    fclose(fp);

    *C = calloc(*N * *N, sizeof(*C));
    assert(*C != NULL);
}

/*
 * Multiply arrays A, B sequentially and accumulate result in C.
 */
void matrix_mult(int N, int *A, int *B, int *C)
{
    int i, j, k;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            for (k = 0; k < N; ++k) {
                C[i*N+j] += A[i*N+k] * B[k*N+j];
            }
        }
    }
}

/*
 * Read N and two N x N integer matrices from file.
 */
void matrix_read(FILE *fp, int *N, int **A, int **B)
{ 
    int i;
    int N_squared = 0;

    fscanf(fp, "%d", N);
    assert((N != NULL) && (*N >= 1) && (*N <= 1000));
    N_squared = *N * *N;
    
    *A = calloc(N_squared, sizeof(**A));
    assert(*A != NULL);

    *B = calloc(N_squared, sizeof(**B));
    assert(*B != NULL);

    i = 0;
    while(i < N_squared) {
        fscanf(fp, "%d", &(*A)[i]);
        ++i;
    }

    i = 0;
    while(i < N_squared) {
        fscanf(fp, "%d", &(*B)[i]);
        ++i;
    }
}

/*
 * Print a matrix.
 */
void matrix_print(const char *desc, int N, int *A)
{
    printf("---- %s ----\n", desc);
    int i, j;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            printf("%5d%c", A[i*N+j], (j == N-1) ? '\n' : ' ');
        }
    }
}
