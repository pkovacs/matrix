## Matrix parallel processing implementations

### Cannon's Generalized Algorithm (MPI)

To build:

    # Install an MPI implementation and its development headers/libraries,
    # e.g. Open MPI

    $ git clone git@github.com:pkovacs/matrix.git
    $ cd matrix
    $ mkdir build
    $ cd build
    $ cmake ..
    or 
    $ cmake -DCMAKE_DEBUG_BUILD ..
    $ make
    or 
    $ make VERBOSE=1

To run on a single process:

    $ cannon/cannon -m ../test/6x6.txt

Note that running the program with 1 process only causes it to use sequential
multiplication which can be useful for reference comparision to the parallel
processing result when running with more than 1 process.

To run with mpirun, possible across several nodes:

    # Ensure your mpi installations are valid and identical on all nodes.
    # Ensure the program binary exists along the same path on all nodes. 
    # Only the rank 0 starting process needs access to the matrix file.

    $ mpirun -np 1 cannon/cannon ../test/6x6.txt
    $ mpirun -np 4 cannon/cannon ../test/6x6.txt
    $ mpirun -np 9 cannon/cannon ../test/6x6.txt
    $ mpirun -np 36 cannon/cannon ../test/6x6.txt

    are all valid process counts for that matrix.

To split across explicit hosts:

    $ mpirun --host <node0>:2,<node1>:2 cannon/cannon ../test/6x6.txt
    $ mpirun --host <node0>:3,<node1>:6 cannon/cannon ../test/6x6.txt
    etc., or use a hosts file.

To run with slurm:

    $ srun -n 4 --mpi=pmix cannon/cannon ../test/6x6.txt
    $ srun -n 36 --mpi=pmix cannon/cannon ../test/6x6.txt
    etc.

The program verifies the validity of the process count and may complain 
with helpful messages such as:

    Number of processes (2) is not a perfect square

    or

    Cannot partition 16x16 matrices on 9 processes. Ensure N/SQRT(processes) is an integral number. 

Typical output would look like this:

     Partitioned the 4x4 matrices on 4 processes of 2x2 each.
     ---- Matrix A ----
      1     2     3     4
      5     6     7     8
      9    10    11    12
     13    14    15    16
     ---- Matrix B ----
      1     5     9    13
      2     6    10    14
      3     7    11    15
      4     8    12    16
     ---- Matrix C ----
     30    70   110   150
     70   174   278   382
    110   278   446   614
    150   382   614   846
