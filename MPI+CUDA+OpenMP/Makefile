build:
	mpicxx -fopenmp -c exe3.c -o exe3.o
	nvcc -I./Common -gencode arch=compute_61,code=sm_61 -c funcForCuda.cu -o funcForCuda.o
	mpicxx -fopenmp -o exe3 exe3.o funcForCuda.o -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart

clean:
	rm -f *.o ./exe3

run:
	mpiexec -np 2 ./exe3


