/* File:       
 *    life.c
 *
 * Author: Chris Kolegraff
 * Class: Parallel CSC410 F2016
 *
 * Purpose:    
 *     To create a parallel program that utilizes the Message Passing Interface to simulate Conway's Game of Life.
 *
 * Compile:    
 *    mpicc -g -Wall -lm -o life life.c
 *    Or use the command 'make'
 *
 * Usage:        
 *    mpiexec -np <number of processes> -hostfile <list of hosts> ./life i j k m n
 *
 * Input:      
 *    i - Number of living cells in the first iteration of the Game of Life.
 *    j - Number of iterations to simulate.
 *    k - The frequency of which the world field is printed to the terminal.
 *    m - Number of rows in the world field matrix.
 *    n - Number of columns in the world field matrix.
 *
 * Output:     
 *    Terminal output detailing the map of the world field. A '.' is a dead cell, and an 'O' is a living one.
 *
 * Algorithm:  
 *    
 *    1. Generate m x n matrix, distribute rows to processes.
 *    2. Exchange rows with neighbors.
 *    3. Compute next state of each process' rows.
 *    4. Set new rows and current rows.
 *    5. If it's time to output, output matrix to terminal. 
 *    6. Repeat steps 2-6 until all iterations have been calculated.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

int* generateWorld(int rows, int cols, int liveCells);
void copyWorldToRows(int *inp, int* out, int proc, int commSize, int numRows,
                 int numCols);
void outputWorld(int* rows, int numRows, int numCols, int commSize, int rank, 
                 int step);
int topRank(int myRank, int commSize);
int botRank(int myRank, int commSize);
void exchangeRows(int *rows, int *topTemp, int *botTemp, int myRank, int numRows, 
                  int numCols, int commSize);
int countNeighbours(int*rows, int *topTemp, int *botTemp, int j, int k, 
                    int numCols);
int lastProc(int numRows, int commSize);
void handleGeneralCase(int *rows, int *topTemp, int *botTemp, int myRank, 
    int size, int top, int bot, int commSize, int numCols);
int getLastValidRow(int *row, int numCols);
void processArgs(int argc, char *argv[], int myRank, int *totalCells, int *totalSteps,
                 int *frequency, int *numRows, int *numCols);


int main(int argc, char *argv[]) {
    int commSize, myRank, totalSteps, frequency;
    int numRows, numCols, count, totalCells;
    int i,j, k;
    int *world = NULL;
    int data[3];
    int *rows = NULL, *botTemp = NULL, *topTemp = NULL, *setTemp = NULL;
    int groups, size;

    // Initialize
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    // Get variable data from command line arguments
    processArgs(argc, argv, myRank, &totalCells, &totalSteps, &frequency,
                &numRows, &numCols);


    // Rank 0 sends size data to other processes
    if(myRank == 0){
        world = generateWorld(numRows, numCols, totalCells);

        // Broadcast data to other processes
        if(numRows > 0 && numCols > 0){
            data[0] = numRows; data[1] = numCols;
            MPI_Bcast(data, 2, MPI_INT, 0, MPI_COMM_WORLD);
        }
    }
    else{
        // Receive data from proc 0
        MPI_Bcast(data, 2, MPI_INT, 0, MPI_COMM_WORLD);
        numRows = data[0]; numCols = data[1];
    }
    
    
    size = numCols * sizeof(int);
    groups = numRows / commSize + 1;
    // Allocate memory for max possible rows ((numRows / commSize) + 1)
    rows = malloc(groups * size);
    
    for(i = 0; i < numCols * groups; i++) rows[i] = 2;

    // Distribute rows and send rows to processes
    if(myRank == 0){
        // Select which process to send to first, do rank 0's last
        for(i = 1; i < commSize; i++){
            // Select which rows each process gets
            copyWorldToRows(world, rows, i, commSize, numRows, numCols);
            // Send rows to process
            MPI_Send(rows, numCols * groups, MPI_INT, i, 0, 
                     MPI_COMM_WORLD);
        }

        copyWorldToRows(world, rows, 0, commSize, numRows, numCols);
    }
    else{
       // Receive rows from proc 0
       MPI_Recv(rows, numCols * groups, MPI_INT, 0, 0, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    topTemp = malloc(groups * size);
    botTemp = malloc(groups * size);
    setTemp = malloc(groups * size);
    for(i = 0; i < numCols * groups; i++){
        setTemp[i] = 2;
        botTemp[i] = 2;
        topTemp[i] = 2;
    }
    
     // Output initial field
    outputWorld(rows, numRows, numCols, commSize, myRank, 0);

    // Calculate Game of Life steps, output dictated by frequency
    for(i = 1; i < totalSteps; i++){
        // Get the procs' rows above and below each proc
        exchangeRows(rows, topTemp, botTemp, myRank, numRows, 
                     numCols, commSize);
                     
        // Calculate next step, j selects row, k selects column
        for(j = 0; j < numCols * groups && rows[j] != 2; j += numCols){
            for(k = 0; k < numCols; k++){
                // Count neighbors from top -> down, left -> right
                count = countNeighbours(rows, topTemp, botTemp, j, k, numCols);

                // Decide whether alive or dead
                if(rows[j+k] == 0){
                    if(count == 3) setTemp[j+k] = 1;
                    else setTemp[j+k] = 0;
                }
                else if(rows[j+k] == 1){
                    if(count == 2 || count == 3) setTemp[j+k] = 1;
                    else setTemp[j+k] = 0;
                }
            }
        }

        // Set calculated row as the current row
        memcpy(&rows[0], &setTemp[0], size * groups);

        // Output if it is time
        if(i % frequency == 0)
            outputWorld(rows, numRows, numCols, commSize, myRank, i);
    }

    // Clean-up / Finalize
    if(world != NULL) free(world);
    if(rows != NULL) free(rows);
    if(topTemp != NULL) free(topTemp);
    if(botTemp != NULL) free(botTemp);
    if(setTemp != NULL) free(setTemp);
    MPI_Finalize();
    return 0;
}

int* generateWorld(int rows, int cols, int liveCells){
    int *temp = malloc(rows * cols * sizeof(int));
    srand(time(NULL));
    //srand(0);
    int i;

    for(i = 0; i < rows*cols; i++) temp[i] = 0;

    if(liveCells > rows*cols) liveCells = rows*cols;
    
    while(liveCells > 0){
        i = rand() % (rows * cols); // Choose random spot in matrix
        // Randomly decide to insert a 1 or not
        if(rand() % 3 == 0){
            while(temp[i] == 1) i = (i+1) % (rows*cols);
            temp[i] = 1;
            liveCells--;
        }
    }
    
    
    // https://bitstorm.org/gameoflife/ Game of Life Comparison
    
    // Use 13 rows, 20 columns, "10 cell row"
    //for(i = 0; i < 10; i++) temp[145 + i] = 1;
    
    
    /*
    // Use 20 rows, 30 columns, "Exploder"
    int start = 7 * cols + rows/2;
    for(i = 0; i < 5; i++){
        temp[start + i*cols] = 1;
        temp[start + i*cols + 4] = 1;
    }
    temp[start+2] = temp[start+2 + 4*cols] = 1;
    */
    
    
    // Use 15 rows, 40 columns, "Glider"
    /*
    int start = 5*cols + rows/2;
    temp[start+1] = temp[start+cols+2] = temp[start+2*cols] = temp[start+2*cols+1] =
    temp[start+2*cols+2] = 1;
    */
    
    
    /*
    // Use 15 rows, 40 columns, "Lightweight Spaceship"
    int start = 5*cols + rows/2;
    temp[start+1] = temp[start+2] = temp[start+3] = temp[start+4] = 
    temp[start+cols] = temp[start+cols+4] =
    temp[start+2*cols+4] =
    temp[start+3*cols] = temp[start+3*cols+3] = 1;
    */
    
    return temp;
}

int countNeighbours(int*rows, int *topTemp, int *botTemp, int j, int k,
                    int numCols){
    int count = 0;// j selects which row, k selects which column in that row
    if(k == 0){
        count += topTemp[j+numCols-1] + topTemp[j] + topTemp[j+1] +
                 rows[j+numCols-1] + rows[j+1] +
                 botTemp[j+numCols-1] + botTemp[j] + botTemp[j+1];
    }
    else if(k == numCols-1){
        count += topTemp[j+numCols-2] + topTemp[j+numCols-1] + topTemp[j] + 
                 rows[j+numCols-2] + rows[j] +
                 botTemp[j+numCols-2] + botTemp[j+numCols-1] + botTemp[j];
    }
    else{
        count += topTemp[j+k-1] + topTemp[j+k] + topTemp[j+k+1] +
                 rows[j+k-1] + rows[j+k+1] +
                 botTemp[j+k-1] + botTemp[j+k] + botTemp[j+k+1];
    }
    return count;
}

// Decide which process owns the last row in the world
int lastProc(int numRows, int commSize){
    if(commSize == 1) return 0;
    if(commSize >= numRows) return numRows-1;
    if(numRows % commSize == 0) return commSize - 1;
    return (numRows % commSize) - 1;
}

void handleGeneralCase(int *rows, int *topTemp, int *botTemp, int myRank, 
    int size, int top, int bot, int commSize, int numCols){
    // Odd and Even amounts of procs require different swapping algorithms
    if(commSize % 2 == 1){
        // Evens receive bottom, odds send top, rank 0 sends to commSize-1
        if((myRank % 2 == 1) || (myRank == 0)){
            MPI_Send(rows, size, MPI_INT, top, 0, MPI_COMM_WORLD);
        }
        if(myRank % 2 == 0){
            MPI_Recv(botTemp, size, MPI_INT, bot, 0, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // Evens receive top, odds send bot, commSize-1 sends to 0
        if((myRank % 2 == 1) || (myRank == commSize - 1)){
            MPI_Send(rows, size, MPI_INT, bot, 0, MPI_COMM_WORLD);
        }
        if(myRank % 2 == 0){
            MPI_Recv(topTemp, size, MPI_INT, top, 0, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Odds receive bot, 0 doesn't send
        if((myRank % 2 == 0) && myRank > 0){
            MPI_Send(rows, size, MPI_INT, top, 0, MPI_COMM_WORLD);
        }
        if(myRank % 2 == 1){
            MPI_Recv(botTemp, size, MPI_INT, bot, 0, MPI_COMM_WORLD, 
                     MPI_STATUS_IGNORE);
        }

        // Odds receive top, commSize-1 doesn't send
        if((myRank % 2 == 0) && myRank < commSize - 1){
            MPI_Send(rows, size, MPI_INT, bot, 0, MPI_COMM_WORLD);
        }
        if(myRank % 2 == 1){
            MPI_Recv(topTemp, size, MPI_INT, top, 0, MPI_COMM_WORLD, 
                     MPI_STATUS_IGNORE);
        }
    }
    else{
        // Even ranks send rows to top and bottom processes, odds receive
        if(myRank % 2 == 0){
            MPI_Send(rows, size, MPI_INT, top, 0, MPI_COMM_WORLD);
            MPI_Send(rows, size, MPI_INT, bot, 0, MPI_COMM_WORLD);
        }
        else{
            MPI_Recv(botTemp, size, MPI_INT, bot, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            MPI_Recv(topTemp, size, MPI_INT, top, 0, MPI_COMM_WORLD, 
                     MPI_STATUS_IGNORE);
        }

        // Odd ranks send rows, evens receive
        if(myRank % 2 == 1){
            MPI_Send(rows, size, MPI_INT, top, 0, MPI_COMM_WORLD);
            MPI_Send(rows, size, MPI_INT, bot, 0, MPI_COMM_WORLD);
        }
        else{
            MPI_Recv(botTemp, size, MPI_INT, bot, 0, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(topTemp, size, MPI_INT, top, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
}

int getLastValidRow(int *row, int numCols){
    int count = 0;

    if(row != NULL){
        while(row[count] != 2) count += numCols;
        count -= numCols;
        return count;
    }
    return 0;
}

void exchangeRows(int *rows, int *topTemp, int *botTemp, int myRank, int numRows, 
                  int numCols, int commSize){
    int *temp = NULL, *temp2 = NULL;
    int j;

    int groups = (numRows / commSize + 1);
    int size = numCols * sizeof(int);
    
    // Don't need to call the send/receive functions if there is one proc
    // Just use the rows array to set the top and bottom arrays
    if(commSize > 1){   
        handleGeneralCase(rows, topTemp, botTemp, myRank, numCols*groups, 
                          topRank(myRank, commSize), botRank(myRank, commSize),
                          commSize, numCols);
    }

    if(commSize == 1){
        // Copy groups, shift to right, copy last group to first spot
        memcpy(&topTemp[numCols], &rows[0], (groups-1)*size);
        memcpy(&topTemp[0], &rows[getLastValidRow(rows, numCols)], size);
        
        // Copy groups, shift to left, copy first initial group to last valid spot
        memcpy(&botTemp[0], &rows[numCols], (groups-1)*size);
        memcpy(&botTemp[getLastValidRow(rows, numCols)], &rows[0], size);
        return;
    }

    // The following special cases are meant to allign the topTemp and botTemp arrays
    // with the rows array in order to make counting live cells easier.
   if(numRows % commSize == 0){
        // Rotate toptemp one group to the right
        temp = malloc(groups*size);
        if(myRank == 0){
            j = getLastValidRow(rows, numCols);
            memcpy(&temp[numCols], &topTemp[0], (groups-1)*size);
            memcpy(&temp[0], &topTemp[j], size);
            memcpy(&topTemp[0], &temp[0], (groups-1)*size);
        }
        else if(myRank == lastProc(numRows, commSize)){
            j = getLastValidRow(rows, numCols);
            memcpy(&botTemp[j+numCols], &botTemp[0], size);
            memcpy(&botTemp[0], &botTemp[numCols], (groups-1)*size);
        }
        free(temp);
        return;
    }
    else{
        if(myRank == lastProc(numRows, commSize)){
            // The last proc will send the last row to rank 0
            // In some cases 0 won't have access to the last row
            temp = malloc(size); 
            memcpy(&temp[0], &rows[(groups-1)*numCols], size);
            MPI_Send(temp, numCols, MPI_INT, 0, 0, MPI_COMM_WORLD);

            MPI_Recv(temp, numCols, MPI_INT, 0, 0, MPI_COMM_WORLD, 
                     MPI_STATUS_IGNORE);
            memcpy(&botTemp[(groups-1)*numCols], &temp[0], size);
            
            free(temp);
        }

        else if(myRank == 0){
            // Receive bottom row from lastproc, copy into to temp
            temp = malloc(size);
            temp2 =  malloc(groups*size);
            MPI_Recv(temp, numCols, MPI_INT, lastProc(numRows, commSize), 
                     0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            memcpy(&temp2[0], &temp[0], size);
            memcpy(&temp2[numCols], &topTemp[0], (groups-1)*size);
            memcpy(&topTemp[0], &temp2[0], groups*size);
            

            // Rank 0 will send the top row to lastproc
            memcpy(&temp[0], &rows[0], size);
            MPI_Send(temp, numCols, MPI_INT, lastProc(numRows, commSize), 0, 
                     MPI_COMM_WORLD);
            free(temp);
            free(temp2);
        }

        if(myRank == commSize-1 && myRank != lastProc(numRows, commSize)){
            // Shift botTemp to the left 1 group
            temp = malloc(size);
            memcpy(&temp[0], &botTemp[0], size);
            memcpy(&botTemp[0], &botTemp[numCols], (groups-1)*size);
            memcpy(&botTemp[(groups-1)*numCols], &temp[0], size);
            free(temp);
        }

        if(myRank == 0 && lastProc(numRows, commSize) == 0){
            temp = malloc(size);
            temp2 = malloc(groups*size);

            memcpy(&botTemp[(groups-1)*numCols], &rows[0], size);
            memcpy(&temp2[0], &topTemp[0], (groups-1)*size);
            memcpy(&topTemp[0], &rows[(groups-1)*numCols], size);
            memcpy(&topTemp[numCols], &temp2[0], (groups-1)*size);
            free(temp);
            free(temp2);
        }
    }
}

void outputWorld(int* rows, int numRows, int numCols, int commSize, int rank, int step)
{
    int size = numCols * sizeof(int);
    int groups = (numRows / commSize + 1);
    
    int* temp = malloc(groups * size);
    int* outArray = malloc(numRows * size);
    int i, j, count;

    if(rank == 0){
        // Receive rows from processes, if p==1, this doesn't run
        for(i = 1; i < commSize; i++){
            MPI_Recv(temp, numCols * groups, MPI_INT, i, i,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            count = 0;
            for( j = i; j < numRows; j += commSize){
                memcpy(&outArray[j*numCols], &temp[count++ * numCols], size);
            }
        }

        count = 0;
        for(j = 0; j < numRows; j += commSize){
            memcpy(&outArray[j*numCols], &rows[count++ * numCols], size);
        }
        
        printf("Step: %d\n", step);
        for(i = 0; i < numRows; i++){
            for(j = 0; j < numCols; j++){
                if(outArray[i*numCols + j] == 1) printf("O ");
                else printf(". ");
            }
            printf("\n");
        }
        for(i = 0; i < numCols; i++) printf("--");
        printf("\n");
    }
    else{
        // Send row information to proc 0
        MPI_Send(rows, numCols * groups, MPI_INT, 0, rank,
                 MPI_COMM_WORLD);
    }

    free(temp);
    free(outArray);
}

void copyWorldToRows(int *inp, int* out, int proc, int commSize, int numRows, int numCols){
    int count = 0, j = 0, m = 0;
    for(j = proc; j < numRows + 1; j += commSize){
        if(j < numRows)
            memcpy(&out[count++ * numCols], &inp[j * numCols], numCols * sizeof(int));
        else{
            // Row is not included in the GameOfLife world, invalidate the row
            for(m = 0; m < numCols; m++) out[count*numCols + m] = 2;
        }
    }
}

int topRank(int myRank, int commSize){
    if(myRank == 0) return commSize - 1;
    return myRank - 1;
}

int botRank(int myRank, int commSize){
    if(myRank == commSize - 1) return 0;
    return myRank + 1;
}


void processArgs(int argc, char *argv[], int myRank, int *totalCells, int *totalSteps,
                 int *frequency, int *numRows, int *numCols){
   if(argc == 6){
        *totalCells = atoi(argv[1]);
        *totalSteps = atoi(argv[2]);
        *frequency = atoi(argv[3]);
        *numRows = atoi(argv[4]);
        *numCols = atoi(argv[5]);
    }
    else if(myRank == 0){
        printf("Usage: ./main i j k m n\n\
i is the starting # of cells\n\
j is the # of iterations\n\
k is the frequency of the output of the field\n\
m, n are the # of rows and columns\n\
Press ctrl+c to exit\n");

        *totalCells = 0;
        *totalSteps = 0;
        *frequency = 0;
        *numRows = 0;
        *numCols = 0;
    } 
}



