#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <ctype.h>


#define MAX 255
enum Search {NOT_FOUND, FOUND, STOP_SEARCH};



//build even and odd substring for sending to the neightbours. we know that original string size is even
void buildStrings(char* evenSubStr, char* oddSubStr, char* originalStr) {

	int size = strlen(originalStr); 
	for(int i = 0; i < size-1 ; i+=2) {
		evenSubStr[i/2] = originalStr[i];
		oddSubStr[i/2] = originalStr[i+1];
	}
	evenSubStr[size/2] = '\0';
	oddSubStr[size/2] = '\0';
}

//function checks if one string is a substring of another (NOT case-sensitive) and without changing original strings
int substringIsFound(char *current, char *sub) {
	int curLen = strlen(current);
	int subLen = strlen(sub);
	
	char tempStr[curLen + 1];
	char tempSub[subLen + 1];
	
	strcpy(tempStr, current);
	strcpy(tempSub, sub);
	
	for(int i = 0; i <= curLen-subLen; i++) {
		int j;
		for(j = 0; j < subLen; j++) {
			if(tolower(tempSub[j]) != tolower(tempStr[i+j]))
				break;
		}
		if(j == subLen)
			return 1;
	}
	return 0;
}


int main(int argc,char** argv) {

        fflush(stdout);
	//initiliaze the variabels 
	int k, N, MaxIterations;
	int rank, size;
	int countProccess = 0;
     	int dim[2], period[2], reorder;
     	int coord[2];
	int found = NOT_FOUND;
	
	//define line, len and read for help in reading from file
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	
		
	MPI_Comm comm;
	MPI_Status status;
		
	MPI_Init(&argc, &argv);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	FILE *f = fopen("data.txt" , "r");
	if (f == NULL){
    		printf("File does not exists\n");
    		return 0;
	}
	fscanf(f,"%d %d %d",&k,&N,&MaxIterations);
	
	char currentString[2*N+1];
	char subString[2*N+1];        //substring for searching
	int foundArr[size];           //for found statuses gathering from each proccess
	char finalStrings[k*k*2*N+1]; //for final gathering in proccess 0
    	dim[0] = k; 
	dim[1] = k;
     	period[0] = 1; 
	period[1] = 1;
     	reorder = 0;
     	
	fscanf(f,"%s",subString); 
	subString[strlen(subString)+1] = '\0';   
    	
    	if (rank == 0)
    	{
    		//need to reead untill new line before we start to read strings for processes
    		getline(&line, &len, f);
    		//define size of buffer string for reading: number of chars + CR + LF + '\n'
    		char buffer[2*N+3];
    		    	
		while(fgets(buffer, sizeof(buffer), f) != NULL) {

			buffer[2*N] = '\0';
			//save first string for itslf (proccess 0)
        		if(countProccess  == 0){
        			strcpy(currentString,buffer);
        			countProccess++;
        		}
        		else {
        			MPI_Send(buffer, 2*N+1, MPI_CHAR, countProccess++, 0, MPI_COMM_WORLD);        			
        		}        		
		 }
    	}
    	else {
    		fflush(stdout);
    		MPI_Recv(currentString, 2*N+1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
    	}
    	//create cartesian topology
	int left_rank, right_rank, up_rank, down_rank;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm);		
	MPI_Cart_shift( comm, 1, 1, &down_rank, &up_rank );
	MPI_Cart_shift( comm, 0, 1, &left_rank, &right_rank );
     	MPI_Cart_coords(comm, rank, 2, coord);
	
        char evenStr[N+1];
        char oddStr[N+1];
        
	for(int iteration = 0; iteration < MaxIterations; iteration++)  ///MaxIterations
	{
		//build string from even chars to the up neightbour and from odd chars to the left neightbour  
     		buildStrings(evenStr, oddStr, currentString);     		
     		MPI_Sendrecv(evenStr, N, MPI_CHAR, left_rank, 0, currentString, N, MPI_CHAR, right_rank, 0, MPI_COMM_WORLD, &status);
     		MPI_Sendrecv(oddStr, N, MPI_CHAR, up_rank, 0, &currentString[N], N, MPI_CHAR, down_rank, 0, MPI_COMM_WORLD, &status);
     		currentString[2*N+1] = '\0'; //add '\0' for normal work with string functions
     		
     		//check for substring
     		if(substringIsFound(currentString, subString)) {
			found = FOUND;
    		}
    		
    		MPI_Gather(&found, 1, MPI_INT, foundArr, 1, MPI_INT, 0,  comm);    		
    		
    		if(rank == 0) { //check if someone has found substring
			  for(int j = 0; j <size; j++) {	        	  	
	        	  	if(foundArr[j] == FOUND) {
	        	  		found = STOP_SEARCH;
	        	  		MPI_Bcast(&found, 1, MPI_INT, 0, MPI_COMM_WORLD);
	        	  		break;
	        	  	}
	        	} 
	        }
	        
	        //TELL TO EVERYONE THAT SOMEBODY HAS FOUND A SUBSTRING ANF GATHER ALL STRINGS FOR PRINTING
	        MPI_Bcast(&found, 1, MPI_INT, 0, MPI_COMM_WORLD);
	        if (found == STOP_SEARCH) {	        
	        	//for better printing, we add '\n' to every string
	                currentString[strlen(currentString)] = '\n';	                
	        	MPI_Gather(currentString, 2*N+1, MPI_CHAR, finalStrings, 2*N+1, MPI_CHAR, 0, comm);
	        	break;
	        }   
	        	
	}
    	if(rank == 0) {
    		if (found == STOP_SEARCH) {    			
    			printf("%s\n", finalStrings);
    		} else {
    			printf("The string was not found\n");
    		}    	
    	}     
     	free(line);
     	MPI_Finalize();
     	fclose(f);
     }
