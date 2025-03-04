#include <stdio.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
//#include "proto.h"
#define HISTSIZE 256

void freeCuda(int* arr)
{
	cudaError_t err = cudaFree(arr);
	if (err != cudaSuccess) {
    		fprintf(stderr, "Failed to free device vector A (error code %s)!\n",
            	cudaGetErrorString(err));
    		exit(EXIT_FAILURE);
	}
}

__global__ void histArr(const int* mainArr,int* d_temp, int numOfElementsPerThread, int range)
{
	int id = blockDim.x * blockIdx.x + threadIdx.x;
  	int offset_Arr = id*numOfElementsPerThread;  // Start of the part of mainArr for this thread
  	int offset_temp = id*range; // Start of the part of temp for this thread
  	int index;

  	// Jump to the place in main array and update the proper part of the temp array
  	for (int i = 0;   i < numOfElementsPerThread;  i++) {
    		 index = mainArr[offset_Arr + i];
   		 d_temp[offset_temp + index]++;
  	}	
}


// Initialize the temporary array
__global__ void initTemp(int* arr) {

  int i;
  int thread_id = blockIdx.x*blockDim.x+threadIdx.x;  
  for (i = 0;   i < HISTSIZE;   i++)   // Each of 200 threads initialize 256 members in temp
    arr[thread_id*HISTSIZE + i] = 0;  

}

// Unify all values in the temp
__global__ void collectResults(int *d_temp, int *d_histo, int part) {
  
  int index = threadIdx.x;  
  int result = 0, i;
  for (i = 0;  i < part; i++) {
  	result += d_temp[index + HISTSIZE*i];  	
  }
  d_histo[index] = result;

}

int startCudaTask(int *h_mainArr, int* h_hist, int numElements, int range) {
	
	cudaError_t err = cudaSuccess;
	int threadsPerBlock = 20;
	int blocksPerGrid = 10;
	
        size_t size = numElements * sizeof(int);   
        
  	int *d_mainArr = NULL;
  	err = cudaMalloc((void **)&d_mainArr, size);
  	if (err != cudaSuccess) {
   		 fprintf(stderr, "Failed to allocate device d_mainArr (error code %s)!\n",
           		 cudaGetErrorString(err));
    		 exit(EXIT_FAILURE);
  	}
  	
	//Temp for histograms for each thread - each thread will calculate it part 
        int *d_temp = NULL;
  	err = cudaMalloc((void **)&d_temp, HISTSIZE*threadsPerBlock*blocksPerGrid*sizeof(int));
  	if (err != cudaSuccess) {
    		fprintf(stderr, "Failed to allocate device d_temp(error code %s)!\n",
        	    cudaGetErrorString(err));
   		exit(EXIT_FAILURE);
  	}

  	// Allocate histo on device
  	int *d_hist = NULL;
  	err = cudaMalloc((void **)&d_hist, HISTSIZE*sizeof(int));
  	if (err != cudaSuccess) {
    		fprintf(stderr, "Failed to allocate device d_hist (error code %s)!\n",
            		cudaGetErrorString(err));
    		exit(EXIT_FAILURE);
  	}

  	err = cudaMemcpy(d_mainArr, h_mainArr, size, cudaMemcpyHostToDevice);
  	  	
  	initTemp<<<blocksPerGrid, threadsPerBlock>>>(d_temp);
  	err = cudaGetLastError();
      	if (err != cudaSuccess) {
    		fprintf(stderr, "Failed to launch initTemp kernel (error code %s)!\n",
            		cudaGetErrorString(err));
    		exit(EXIT_FAILURE);
  	}

     	//Calculate the part for each thread in main array
  	int elementsPerThread = numElements/(threadsPerBlock*blocksPerGrid);
  	histArr<<<blocksPerGrid, threadsPerBlock>>>(d_mainArr, d_temp, elementsPerThread, HISTSIZE);
	err = cudaGetLastError();
	if (err != cudaSuccess) {
    		fprintf(stderr, "Failed to launch histArr kernel (error code %s)!\n",
          	  cudaGetErrorString(err));
    		exit(EXIT_FAILURE);
	}
	//cudaDeviceSynchronize();
	collectResults<<<1, HISTSIZE>>>(d_temp, d_hist, threadsPerBlock*blocksPerGrid);
	err = cudaGetLastError();
	if (err != cudaSuccess) {
    		fprintf(stderr, "Failed to launch collectResults kernel (error code %s)!\n",
          	  cudaGetErrorString(err));
    		exit(EXIT_FAILURE);
	}

	// Copy the  result from GPU to the host memory.
        err = cudaMemcpy(h_hist, d_hist, HISTSIZE*sizeof(int), cudaMemcpyDeviceToHost);
  	if (err != cudaSuccess) {
    		fprintf(stderr, "133: Failed to copy memory from device to host (error code %s)!\n",
            		cudaGetErrorString(err));     			
            	exit(EXIT_FAILURE);
  	}  
  		  
  	freeCuda(d_mainArr);
  	freeCuda(d_temp);
  	freeCuda(d_hist);
  	return 1;
}


