//
//  main.cpp
//  D_Sanchez_FinalProject
//  Created by Daniel Sanchez, CWID: 892807090.
//
// Summary: This program uses MPI to calculate the largest 2x2 submatrix given a larger nxn (n>=10) matrix. Uses blocking message passing.
//
// Problem Statement:
// Given a very large (n>10) matrix of n x n elements, write a distributed
// program in MPI that outputs a submatrix of size 2x2 with the largest sum of its elements and
// its relative position or displacement (row-column of the top left element) in the original matrix.
// There will be only one process with rank 0, that will read the file name with the input data, read
// from the file the value of n, read from the file the entire matrix, and will output the submatrix of
// size 2x2 with the largest sum of its elements and its position on the screen. All the other processes
// will receive portions of the matrix and contribute to identifying the submatrix.


#include <iostream>
#include <mpi.h>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;
//need a function to read a matrix from a file. This matrix must be a square matrix.
void readMatrixFromTextFile(string fileName, int n, int** matrix);
//display a matrix
void displayMatrix(int** matrix, int n);
//calculate sum of submatrix
int calculateSum(int subMatrix[2][2]);
//create a 2x2 subMatrix
void createSubMatrix(int** matrix, int x, int y, int subMatrix[2][2]);

int main(int argc, char* argv[]) { //main accepts the filename and reads the value of N and the Matrix itself from the file.
  int numberOfProcesses = 0; //the total number of processes
  int rank = 0; //rank of process

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
  int n;
  //must assign at least two processes to demonstrate the uses of MPI
  if(numberOfProcesses < 2)
  {
    cout << "Too few processes. Please assign a minimum of two processes for this program." << endl;
    MPI_Finalize(); //clean up the MPI environment
    return -1; //end the program
  }
  else if(rank == 0) //parent process
  {
    int yResult, xResult; //the coordinates of the largest submatrix

    int** mainMatrix;
    int subMatrix[2][2]; //submatrix to send to child processes
    int** largestSubMatrix = new int*[2]; //the largest found so far
    int rankToSend = 0;
    int largestSum = 0;
    int count;
    int currentSum;
    int rankFoundAt; //the rank of the process that found the largest submatrix
    //get n from the file, and attempt to read in the matrix
    string fileName = argv[1];
    string line;
    //allocate the largest submatrix
    for(int i = 0; i < 2; i++)
    {
      largestSubMatrix[i] = new int[2];
    }
    ifstream matrixFile;
    matrixFile.open(fileName);
     if(matrixFile.is_open())
     {
       //getline(matrixFile, line);
       //n = stoi(line);
       matrixFile  >> n;
       matrixFile.close();
       //initialize the matrix then fill it from the file
       mainMatrix = new int*[n];
       for(int i = 0; i < n; i++)
       {
         mainMatrix[i] = new int[n];
       }
       readMatrixFromTextFile(fileName, n, mainMatrix);

     }
     else
     {
       cout << "Cannot open file. Closing program." << endl;
       MPI_Finalize();
       return -1;
     }
     //generate 2x2 submatrices and send them to worker processes
     for(int i = 0; i < n-1; i++)
     {
       for(int j = 0; j < n-1; j++)
       {

         rankToSend = (count % (numberOfProcesses -1)) + 1; //must at least be 1, can't send to self
         createSubMatrix(mainMatrix, i, j, subMatrix);
         MPI_Send(subMatrix, 4, MPI_INT, rankToSend, rankToSend, MPI_COMM_WORLD); //blocking
         MPI_Recv(&currentSum, 1, MPI_INT, rankToSend, rankToSend, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         if(currentSum > largestSum)
         {
           yResult = j;
           xResult = i;
           largestSubMatrix[0][0] = subMatrix[0][0];
           largestSubMatrix[0][1] = subMatrix[0][1];
           largestSubMatrix[1][0] = subMatrix[1][0];
           largestSubMatrix[1][1] = subMatrix[1][1];
           rankFoundAt = rankToSend;
           largestSum = currentSum;
         }
         count++;
       }
     }
     //display the result
    cout << "Input Matrix:" << endl;
 		displayMatrix(mainMatrix, n);

    cout << "\nThe largest sum is:  " << largestSum << " found at: y = " << xResult << " and x = " << yResult << " found by process rank " << rankFoundAt << endl;
    cout << "The largest submatrix: " << endl;
		displayMatrix(largestSubMatrix, 2);



		//free memory
		delete(mainMatrix);
  }
  else
  {
    int workerMatrix[2][2];
		int result = 0;
		int numOfWorkers = numberOfProcesses - 1;




		for (int x = rank; x <=((n-1)*(n-1)); x += numOfWorkers)
		{

			MPI_Recv(workerMatrix, 4, MPI_INT, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		  result = calculateSum(workerMatrix);
      //send the result to the parent
			MPI_Send(&result, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
		}
  }
    MPI_Finalize();
    return 0;
}
void readMatrixFromTextFile(string fileName, int n, int** matrix)
{

  ifstream readFile;
  readFile.open(fileName);
  string skip;
  string readNum;
  int num;
  getline(readFile, skip);
  for(int i = 0; i < n; i++)
  {
    for(int j = 0; j < n; j++)
    {
      //readFile >> readNum;
      //num = stoi(readNum);
      readFile >> num;
      matrix[i][j] = num;
    }
  }
  readFile.close();
}
int calculateSum(int subMatrix[2][2])
{
  int sum = 0;
  for(int i = 0; i < 2; i++)
  {
    for(int j = 0; j < 2; j++)
    {
      sum += subMatrix[i][j];
    }
  }
  return sum;
}
void createSubMatrix(int** matrix, int x, int y, int subMatrix[2][2])
{
  subMatrix[0][0] = matrix[x][y];
  subMatrix[0][1] = matrix[x][y+1];
  subMatrix[1][0] = matrix[x+1][y];
  subMatrix[1][1] = matrix[x+1][y+1];

}
void displayMatrix(int** matrix, int n) //display a square matrix
{
  for(int i = 0; i < n; i++)
  {
    for(int j = 0; j < n; j++)
    {
      cout << setw(4) << left << matrix[i][j];
    }
    cout << endl;
  }
  cout << endl;
}
