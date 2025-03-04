The purpose of this exercise is to have experience with heterogeneous environment MPI + CUDA + OpenMP

## Problem definition:

Calculate the Histogram of a large array (more than 300000 elements) of randomly defined numbers in a range [0, 255]


## Requirments:

•	Launch two processes and use MPI, OpenMP and CUDA for effective parallel computations.
•	Use 10 blocks of 20 threads for CUDA.
•	Don’t forget to use all the cores on each machine! There were 4 cores on my Ubuntu machine. 
