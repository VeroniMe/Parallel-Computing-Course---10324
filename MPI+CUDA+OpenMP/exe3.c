#include <stdio.h>
#include <omp.h>
#include <mpi.h>
#include <stdlib.h>


#define HISTSIZE 256 
#define PART 100000 
#define SIZE 400000 

int startCudaTask(int *h_mainArr, int* h_hist, int numElements, int range);

void test(int* hist) {
    int sum=0;
    for(int i=0; i<HISTSIZE; i++){
        sum+=hist[i];
    }
    if (sum != SIZE) {
           printf("Wrong Calculations - Failure of the test - sum = %d\n", sum);
           return;
    }
    printf("The test passed successfully, sum= %d\n", sum); 
}



int startOmpTask(int *mainArr, int* h_hist){	
	
	int i, j;
	int numOfTreads = 4;
	omp_set_num_threads(numOfTreads);

	int *temp = (int*) malloc (numOfTreads * HISTSIZE * sizeof(int));
	int *histogram = (int *) malloc (HISTSIZE * sizeof(int));
	if (temp == NULL || histogram == NULL) {
		printf("Cannot allocate the memory\n");
		return 0;
	}	
	

	#pragma omp parallel for
	for (int i = 0;  i < numOfTreads * HISTSIZE; i++)
		temp[i] = 0;	

	// Create the private histogram	
	#pragma omp parallel private (i)
	{
		int tid = omp_get_thread_num();
		int offset = tid*HISTSIZE;
		
		#pragma omp parallel for
		for (i = 0;   i < PART;  i++) {			
			temp[offset + mainArr[i]]++;
		}
	}
	

     	#pragma omp parallel for
	for (i = 0;  i < HISTSIZE; i++)
		histogram[i] = 0;

	// Unify the temporary results
	#pragma omp parallel private(i, j)
	{
		int tid = omp_get_thread_num();
		int part = HISTSIZE/numOfTreads;
		int offset = tid*(part);
		//printf("tid = %d, part = %d\n", tid, part);
		
		#pragma omp parallel for
		for (i = 0;   i < numOfTreads;   i++) {
			int result = 0;
			for (j = 0;  j < part;   j++) {
				h_hist[offset + j] = temp[i*HISTSIZE + offset + j];
				
			}
		}
	}	
	free(temp);
	free(histogram);
	return 1;
}

int main(int argc, char* argv[])
{	
   	 int countOfParts = 1;
   	 MPI_Init(NULL, NULL);
   	 int numProc;
   	 int my_rank;
   	 MPI_Status  status;
   	 MPI_Comm_size(MPI_COMM_WORLD, &numProc);    
   	 MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    
    
    	//Initilise arrays for cuda and omp calculations
	
	int h_histo_cuda[HISTSIZE];	
	int h_histo_omp[HISTSIZE];


	// Allocate the host input array
	int* h_mainArr;
	int histo_test[HISTSIZE];

    	// Divide the tasks between both processes
    	if (my_rank == 0) {
       		// Allocate memory for the whole array and send a half of the array to other process
       		if ((h_mainArr = (int *) malloc(4*PART*sizeof(int))) == NULL)
          		MPI_Abort(MPI_COMM_WORLD, __LINE__);
          
       		for (int i = 0;  i < 4*PART;  i++) {
       			h_mainArr[i] = rand()%HISTSIZE;
     		}
     		
       		MPI_Send(h_mainArr + 2*PART, 2*PART, MPI_INT, 1, 0, MPI_COMM_WORLD);
    	} else {
       		// Allocate memory and reieve a half of array from other process
        	if ((h_mainArr = (int *) malloc(2*PART*sizeof(int))) == NULL)
           		MPI_Abort(MPI_COMM_WORLD, __LINE__);
        	MPI_Recv(h_mainArr, 2*PART, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    	}    	
    	
     	// On each process - perform a first half of its task with OpenMP  
        if (startOmpTask(h_mainArr, h_histo_omp) != 1)
        	MPI_Abort(MPI_COMM_WORLD, __LINE__);	
        	
    	// On each process - perform a second half of its task with CUDA
    	if (startCudaTask(h_mainArr + PART,h_histo_cuda, PART, HISTSIZE) != 1)
        	MPI_Abort(MPI_COMM_WORLD, __LINE__);


	// Unify histo results from CUDA and OpenMP - put all results in h_histo_omp array
	int i, j, numOfTreads, result;
	numOfTreads = 4;
	int part = HISTSIZE/numOfTreads;	
	omp_set_num_threads(numOfTreads);
	#pragma omp parallel private(i)
	{
		int offset = omp_get_thread_num() * part;
		#pragma omp parallel for
		for (i = 0;   i < part;   i++) {
			result = h_histo_cuda[offset + i];
			h_histo_omp[offset + i] = h_histo_omp[offset + i] + result;
		}
	}
	
	//Now we have histogram in h_histo_omp. No we process1 want to get hist from proc2.
	//Proc 1 put this to his h_histo_cuda array
	if (my_rank == 0) 
       		MPI_Recv(h_histo_cuda, HISTSIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    	else 
       		MPI_Send(h_histo_omp, HISTSIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
       		
       	// Process 0 will unify histo results from h_histo_cuda and h_histo_omp - put all results in  h_histo_omp 
       	if (my_rank == 0) {
		int i, j, numOfTreads, result;
		numOfTreads = 4;
		int part = HISTSIZE/numOfTreads;	
		omp_set_num_threads(numOfTreads);
		#pragma omp parallel private(i)
		{
			int offset = omp_get_thread_num() * part;
			#pragma omp parallel for			
			for (i = 0;   i < part;   i++) {
				result = h_histo_cuda[offset + i];
				h_histo_omp[offset + i] = h_histo_omp[offset + i] + result;
			}
		}

	}
	//Function test will check if all calculations are right       
        if(my_rank == 0) {
         	test(h_histo_omp);
         
        }	
	MPI_Finalize();
	free(h_mainArr);
	
}

