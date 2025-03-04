#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define HEAVY  1000
#define SIZE   60

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

int main(int argc, char* argv[]) {
	int numproc;
	int my_rank;
	double sumAnswers;
	double answer;
	int startPoint;
	
	//Realize how much proccesses we have
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numproc);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Status status;
	
	//Calculate the amount of tasks per proccess
	int numOfTasksPerProccess = SIZE/(numproc-1);

	if(my_rank == 0){
		double t1 = MPI_Wtime();	
		for (int x = 1; x < numproc; x++) {
			MPI_Recv(&answer,1,MPI_DOUBLE,x,0,MPI_COMM_WORLD,&status);
			sumAnswers+=answer;
		}
		double t2 = MPI_Wtime();
		printf("Answer = %e, time = %f\n", sumAnswers, (t2-t1));			
	}
	else {
		//Calculate the start index for every proccess
		startPoint = (my_rank-1)*numOfTasksPerProccess;
		for (int x = startPoint; x < startPoint+numOfTasksPerProccess; x++)
			for (int y = 0; y < SIZE; y++)
				answer += heavy(x, y);
		MPI_Send(&answer,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}

