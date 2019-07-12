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

#define AUTO_PTR(free_fn) __attribute__((cleanup (free_fn)))

void initialize(int argc, char *argv[], int *N, int **A, int **B, int **C);
void matrix_read(FILE *fp, int *N, int **A, int **B);
void matrix_print(const char *desc, int N, int *A);
void matrix_shift_row_left(int row, int N, int *A);
void matrix_shift_col_up(int col, int N, int *B);
void matrix_free(int **M);

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
 * Read a file of two square matrices and multiply them in parallel
 * using Cannon's generalized algorithm.  You may assign fewer tasks
 * than N*N, in which case each task handles a submatrix, as long as 
 * N / sqrt(tasks) is an integer value and thus the matrices can be
 * partitioned into equal blocks.  
 */
int main(int argc, char *argv[])
{
    int N = 0;
    AUTO_PTR(matrix_free) int *A = NULL;
    AUTO_PTR(matrix_free) int *B = NULL;
    AUTO_PTR(matrix_free) int *C = NULL;
    int i, j, k;
    int rank, size;
    int rank_left, rank_right, rank_down, rank_up;
    int dims[2];
    int coords[2];
    int local_data[2];
    int local_sum = 0;
    const int periods[2] = { 1, 1 };
    const int reorder = 1;
    const size_t dims_sz = sizeof(dims)/sizeof(dims[0]);
    MPI_Comm cartesian;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        initialize(argc, argv, &N, &A, &B, &C);
        if (size == 1) {
            // Use sequential multiplication if just 1 task
            printf("Using sequential multiplication on 1 task.\n");
            matrix_print("Matrix A", N, A);
            matrix_print("Matrix B", N, B);
            for (i = 0; i < N; ++i) {
                for (j = 0; j < N; ++j) {
                    for (k = 0; k < N; ++k) {
                        C[i*N+j] += A[i*N+k] * B[k*N+j];
                    }
                }
            }
            matrix_print("Matrix C", N, C);
        }
    }

    if (size == 1) {
        MPI_Finalize();
        return 0;
    }


    // Number of tasks must be a perfect square
    double fsize_sqrt = sqrt(size);
    int isize_sqrt = fsize_sqrt;
    if (fsize_sqrt != isize_sqrt) {
        if (rank == 0) {
            fprintf(stderr, "Number of tasks (%d) is not a perfect square\n", size);
        }
        MPI_Finalize();
        return 0;
    }

    // Broadcast N, the matrix dimension, to all tasks
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    dims[0] = N;
    dims[1] = N;

    // Matrices must be partitioned into equal sized blocks
    if (N % isize_sqrt != 0) {
        if (rank == 0) {
            fprintf(stderr, "Cannot partition %dx%d matrices with %d tasks. "
                    "Ensure N/SQRT(tasks) is an integral number.\n", N, N, size);
        }
        MPI_Finalize();
        return 0;
    }

    if (rank == 0) {
        printf("Partitioned the %dx%d matrices over %d tasks.\n", N, N, size);
    }

    // Use a cartesian process topology matching the array dimensions
    MPI_Cart_create(MPI_COMM_WORLD, dims_sz, dims, periods, reorder, &cartesian);
    MPI_Comm_rank(cartesian, &rank);
    MPI_Cart_coords(cartesian, rank, dims_sz, coords);
    MPI_Cart_shift(cartesian, 1, 1, &rank_left, &rank_right);
    MPI_Cart_shift(cartesian, 0, 1, &rank_up, &rank_down);

    if (rank == 0) {
        // Initial shift of ith row of A left by i
        for (i = 0; i < N; ++i) {
            j = i;
            while (j--) {
                matrix_shift_row_left(i, N, A);
            }
        }

        // Initial shift of jth col of B up by j
        for (j = 0; j < N; ++j) {
            i = j;
            while (i--) {
                matrix_shift_col_up(j, N, B);
            }
        }
    }

    if (rank == 0) {
        matrix_print("Matrix A", N, A);
        matrix_print("Matrix B", N, B);
    }

    // Rank 0 scatters all A(i,j) and B(i,j) to local_data of process (i,j)
    MPI_Scatter(A, 1, MPI_INT, &local_data[0], 1, MPI_INT, 0, cartesian);
    MPI_Scatter(B, 1, MPI_INT, &local_data[1], 1, MPI_INT, 0, cartesian);

    // Process (i,j) performs N accumulate/shift on its local data
    for (i = 0; i < N; ++i) {
        // Accumulate the local sum for C(i,j)
        local_sum += local_data[0] * local_data[1];

        // Send local_data[0] left by one and local_data[1] up by one
        MPI_Sendrecv_replace(&local_data[0], 1, MPI_INT, rank_left, 1,
                             rank_right, 1, cartesian, MPI_STATUS_IGNORE);
        MPI_Sendrecv_replace(&local_data[1], 1, MPI_INT, rank_up, 1,
                             rank_down, 1, cartesian, MPI_STATUS_IGNORE);
    }

    // Rank 0 gathers the final sum C(i,j) from each process (i,j)
    MPI_Gather(&local_sum, 1, MPI_INT, C, 1, MPI_INT, 0, cartesian);

    if (rank == 0) {
        matrix_print("Matrix C", N, C);
    }

    MPI_Finalize();
    return 0;
}

/*
 * Initialize is for rank 0 only: parse options, read and set up matrices.
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
 * Read N and two N x N integer matrices from file.
 */
void matrix_read(FILE *fp, int *N, int **A, int **B)
{ 
    int i;
    int N_SQUARED = 0;

    fscanf(fp, "%d", N);
    assert((N != NULL) && (*N >= 1) && (*N <= 1000));
    N_SQUARED = *N * *N;
    
    *A = calloc(N_SQUARED, sizeof(**A));
    assert(*A != NULL);

    *B = calloc(N_SQUARED, sizeof(**B));
    assert(*B != NULL);

    i = 0;
    while(i < N_SQUARED) {
        fscanf(fp, "%d", &(*A)[i]);
        ++i;
    }

    i = 0;
    while(i < N_SQUARED) {
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

/*
 * Shift the matrix row left by 1 with circular wrapping.
 */
void matrix_shift_row_left(int row, int N, int *A)
{
    int j, tmp;
    tmp = A[row*N];
    for (j = 0; j < N-1; ++j) {
        A[row*N+j] = A[row*N+j+1];
    }
    A[row*N+j] = tmp;
}

/*
 * Shift the matrix col up by 1 with circular wrapping.
 */
void matrix_shift_col_up(int col, int N, int *B)
{
    int i, tmp;
    tmp  = B[col];
    for (i = 0; i < N-1; ++i) {
        B[i*N+col] = B[(i+1)*N+col];
    }
    B[i*N+col] = tmp;
}

/*
 * Free the matrix memory when leaving scope.
 */
void matrix_free(int **M)
{
    free (*M);
}
