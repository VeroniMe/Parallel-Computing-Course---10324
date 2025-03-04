#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define HEAVY  1000
#define SIZE   60
#define JOB_TAG 35
#define FINISH_TAG 45

// This function performs heavy computations, 
// its run time depends on x and y values
// DO NOT change the function
double heavy(int x, int y) {
	int i, loop;
	double sum = 0;
	
	if (x > 0.25*SIZE &&  x < 0.5*SIZE && y > 0.4 * SIZE && y < 0.6 * SIZE)
		loop = x * y;
	else
		loop = y + x;

	for (i = 0; i < loop * HEAVY; i++)
		sum += cos(exp(sin((double)i / HEAVY)));

	return  sum;
}

void getTask(int* arrayOfMatrixValues){
	//For the first iteration
        if((arrayOfMatrixValues[0] == -1) && (arrayOfMatrixValues[1] == -1)) {
        	arrayOfMatrixValues[0] = 0;
		arrayOfMatrixValues[1] = 0;  	
	
	//For last iteration
        } else if((arrayOfMatrixValues[0] == SIZE-1) && (arrayOfMatrixValues[1] == SIZE-1)) {
        	return;    
        
        //When we get to the last column , we move to the next row
	} else if(arrayOfMatrixValues[1] == SIZE-1)
	{
		arrayOfMatrixValues[0]++;
		arrayOfMatrixValues[1] = 0;
	}
	else
		arrayOfMatrixValues[1]++;
}
	

int main(int argc, char* argv[]) {
	int numproc;
	int my_rank;
	double sumAnswers;
	double answer;
	int startPoint;
	int arrayOfMatrixValues[2] = {-1, -1}; //init the values for the coordinates for calculation
	int numOfTasks = SIZE*SIZE;
	int pLast; //Last proccess that finish and ask for task
	
	//Realize how much proccesses we have
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numproc);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Status status;
	
	//Calculate the amount of tasks per proccess
	int numOfTasksPerProccess = SIZE/(numproc-1);

	if(my_rank == 0){
		double t1 = MPI_Wtime();
		for(int i = 1 ; i < numproc ; i++) {
			getTask(arrayOfMatrixValues);
			MPI_Send(arrayOfMatrixValues,2,MPI_INT,i,JOB_TAG,MPI_COMM_WORLD);
			numOfTasks--;
		}
		do {
			//wait for answer from any proccess
			MPI_Recv(&answer,1,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&status);
			pLast = status.MPI_SOURCE;
			sumAnswers+=answer;
			if(numOfTasks > 0) {
				getTask(arrayOfMatrixValues);
				MPI_Send(arrayOfMatrixValues,2,MPI_INT,pLast,JOB_TAG,MPI_COMM_WORLD);
				numOfTasks--;
			}
			else {	
				//decrease the number of proccesses until the only proccess that work is the master			
				MPI_Send(arrayOfMatrixValues,2,MPI_INT,pLast,FINISH_TAG,MPI_COMM_WORLD);
				numproc--;
			}
		
		} while(numproc != 1);		
		double t2 = MPI_Wtime();
		printf("Answer = %e, time of run = %f\n", sumAnswers, (t2-t1));		
	}
	else {
		do{
			//Calculate the start index for every proccess
			MPI_Recv(arrayOfMatrixValues,2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			if(status.MPI_TAG == FINISH_TAG)
				break;
			else{
				answer = heavy(arrayOfMatrixValues[0],arrayOfMatrixValues[1]);
				MPI_Send(&answer,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
			}
		} while(status.MPI_TAG != FINISH_TAG);
	}
	
	MPI_Finalize();
	return 0;
}




