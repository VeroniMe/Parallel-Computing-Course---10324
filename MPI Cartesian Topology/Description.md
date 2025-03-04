The purpose of this exercise is to practice MPI Cartesian topology.


# Problem definition

Launch K^2 processes. The process 0 reads K^2 strings from the given file data.txt. Each string has the same length 2*N. The process 0 distributes these strings between all process in such a way that process with rank i receives the i-th string. 
Perform following iteration steps:
1.	Organize the processes using KxK Cartesian two-dimensional topology.
2.	Each process sends the characters from odd places of its current string to the left process and the even located characters to the up process (allow each process to have all four neighbors).
3.	The characters received from the right process became a first part of the current string, The characters received from the down process became a second part of the current string (Iteration i, Iteration i + 1 tables).
4.	Each process searches if the given string SubString presents into the current string. If the answer is positive for at least one process - the program ends and the process 0 displays all string received form each process according to their ranks – in order from the first to the last.
5.	If the SubString was not found by any process – return to the step 2 until the maximum iteration MaxIterations is performed. 
6.	If the SubString  was not found after MaxIterations - the process 0 outputs “The string was not found”.


### Iteration i
||||
|---------|---------|---------|
|         |  a b c d   |   k l m n  |
|         |  x y z w   |         |

### Iteration i + 1
||||
|---------|---------|---------|
|         |  l n x z   |    |
|         |     |         |

Input File structure:
	K    N    MaxIterations
	SubString
	String1
	String2
	String3
	. . .
	StringK2
