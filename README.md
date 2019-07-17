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
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..
    $ make
    or 
    $ make VERBOSE=1

To run on a single process:

    $ cannon/cannon -m ../test/6x6.txt

Note that running the program with 1 process causes it to use sequential
multiplication which can be useful for reference comparision to the parallel
processing result when running with more than 1 process.

To run with mpirun, possibly across several nodes:

    # Ensure your MPI installations are valid and identical on all nodes.
    # Ensure the program binary exists along the same path on all nodes. 
    # Only the rank 0 starting process needs access to the matrix file.

    $ mpirun -np 1 cannon/cannon -m ../test/6x6.txt
    $ mpirun -np 4 cannon/cannon -m ../test/6x6.txt
    $ mpirun -np 9 cannon/cannon -m ../test/6x6.txt
    $ mpirun -np 36 cannon/cannon -m ../test/6x6.txt

    are all valid process counts for that matrix.

To split across explicit hosts:

    $ mpirun --host <node0>:2,<node1>:2 cannon/cannon -m ../test/6x6.txt
    $ mpirun --host <node0>:3,<node1>:6 cannon/cannon -m ../test/6x6.txt
    etc., or use a hosts file.

To run with slurm:

    $ salloc -n 9 sh
    salloc: Granted job allocation XXX
    # distribute matrix file input from this launch point
    $ sbcast -f ../test/6x6.txt /tmp/$$-6x6.txt
    $ srun --mpi=pmix cannon/cannon -m /tmp/$$-6x6.txt
    Partitioned the 6x6 matrices on 9 processes of 2x2 each.
    ... (result) ...
    $ srun rm -f /tmp/$$-6x6.txt
    $ exit
    salloc: Relinquishing job allocation XXX

    etc.

The program verifies the validity of the process count and may complain 
with helpful messages such as:

    Number of processes (7) is not a perfect square

    or

    Cannot partition 16x16 matrices on 9 processes. Ensure N/SQRT(processes) is an integral number. 

Typical output would look like this:

    Partitioned the 6x6 matrices on 9 processes of 2x2 each.
    ---- Matrix A ----
        1     2     3     4     5     6
        7     8     9    10    11    12
       13    14    15    16    17    18
       19    20    21    22    23    24
       25    26    27    28    29    30
       31    32    33    34    35    36
    ---- Matrix B ----
        1     7    13    19    25    31
        2     8    14    20    26    32
        3     9    15    21    27    33
        4    10    16    22    28    34
        5    11    17    23    29    35
        6    12    18    24    30    36
    ---- Matrix C ----
       91   217   343   469   595   721
      217   559   901  1243  1585  1927
      343   901  1459  2017  2575  3133
      469  1243  2017  2791  3565  4339
      595  1585  2575  3565  4555  5545
      721  1927  3133  4339  5545  6751
