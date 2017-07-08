#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <string.h>

using namespace std;

#include <mpi.h>
#include <omp.h>

#include "multimerge.h"
#include "utilities.h"

int main( int argc, char* argv[]) {
    int my_rank, numprocs;
    int thread_count;

    double startwtime = 0.0, endwtime;

    MPI_Init(argc, argv);

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    thread_count = numprocs * 2;

    //		 -DS nnnn to set myDataSize
    //		 -SR nnnn to set randomSeed
    int randomSeed = 1000;
    int myDataSize = 4000;
    for(int i=0;i<argc;i++) {
        if (strcmp(argv[i],"-DS")==0) {
            myDataSize = atoi(argv[i+1]); i++;
        } else if (strcmp(argv[i],"-SR")==0) {
            randomSeed = atoi(argv[i+1]); i++;
        }
    }

    int myData[myDataSize];
    int myDataLengths[numprocs];
    int myDataStarts[numprocs];

    int pivotbuffer[numprocs*numprocs];
    int pivotbufferSize;

#   pragma omp parallel for num_threads(thread_count) default(none) share(myDataLengths, myDataStarts, numprocs) private(i)
    for(int i=0;i<numprocs;i++) {
        myDataLengths[i] = myDataSize/numprocs;
        myDataStarts[i]= i*myDataSize/numprocs;
    }
    myDataLengths[numprocs-1]+=(myDataSize%numprocs);

    if (my_rank == 0) {
        srandom(randomSeed);
#       pragma omp parallel for num_threads(2) default(none) share(myDataSize, myData) private(index)
        for(int index=0; index<myDataSize; index++) {
            myData[index] = random()% 900;
        }

        startwtime = MPI::Wtime();
    }

    if (my_rank==0) {
        MPI::COMM_WORLD.Scatterv(myData, myDataLengths, myDataStarts, MPI::INT, MPI_IN_PLACE, myDataLengths[my_rank], MPI::INT,0);
    } else {
        MPI::COMM_WORLD.Scatterv(myData, myDataLengths, myDataStarts, MPI::INT, myData, myDataLengths[my_rank], MPI::INT, 0);
    }

    qsort(myData,myDataLengths[my_rank], sizeof(int), compare_ints);

    // All processors collect regular samples from sorted list Consider an offset to the myData[] index
#   pragma omp parallel for num_threads(2) default(none) share(numprocs, pivotbuffer, myData, myDataLengths) private(index, my_rank)
    for(int index=0;index<numprocs;index++) {
        pivotbuffer[index]= myData[index*myDataLengths[my_rank]/numprocs];
    }


    if (my_rank==0) {
        MPI::COMM_WORLD.Gather(MPI_IN_PLACE,numprocs, MPI::INT, pivotbuffer, numprocs, MPI::INT, 0);
    } else {
        MPI::COMM_WORLD.Gather(pivotbuffer, numprocs, MPI::INT, pivotbuffer, numprocs, MPI::INT, 0);
    }

    if (my_rank == 0) {
        int *starts[numprocs];
        int lengths[numprocs];
#       pragma omp parallel for num_threads(2) default(none) share(numprocs, starts, pivotbuffer, lengths) private(i)
        for(int i=0;i<numprocs;i++) {
            starts[i]=&pivotbuffer[i*numprocs];
            lengths[i]=numprocs;
        }

        int tempbuffer[numprocs*numprocs];  // merged list
        multimerge(starts,lengths,numprocs,tempbuffer,numprocs*numprocs);

        for(int i=0; i<numprocs-1; i++) {
            pivotbuffer[i] = tempbuffer[(i+1)*numprocs];
        }
    }

    MPI::COMM_WORLD.Bcast(pivotbuffer, numprocs-1, MPI::INT, 0);

    int classStart[numprocs];
    int classLength[numprocs];

    int dataindex=0;
    for(int classindex=0; classindex<numprocs-1; classindex++) {
        classStart[classindex] = dataindex;
        classLength[classindex]=0;

        while((dataindex< myDataLengths[my_rank]) && (myData[dataindex]<=pivotbuffer[classindex])) {
            classLength[classindex]++;
            dataindex++;
        }
    }

    classStart[numprocs-1] = dataindex;
    classLength[numprocs-1] = myDataLengths[my_rank] - dataindex;

    int recvbuffer[myDataSize];
    int recvLengths[numprocs];
    int recvStarts[numprocs];

#   pragma omp parallel for num_threads(2) default(none) share(numprocs, classLength, recvLengths, my_rank, recvStarts, myData, recvbuffer) private(iprocessor)
    for(int iprocessor=0; iprocessor<numprocs; iprocessor++) {
        MPI::COMM_WORLD.Gather(&classLength[iprocessor], 1, MPI::INT, recvLengths, 1, MPI::INT, iprocessor);
        if (my_rank == iprocessor) {
            recvStarts[0]=0;
            for(int i=1;i<numprocs; i++) {
                recvStarts[i] = recvStarts[i-1]+recvLengths[i-1];
            }
        }
        MPI::COMM_WORLD.Gatherv(&myData[classStart[iprocessor]], classLength[iprocessor], MPI::INT, recvbuffer, recvLengths, recvStarts, MPI::INT, iprocessor);
    }

    int *mmStarts[numprocs];

    for(int i=0;i<numprocs;i++) {
        mmStarts[i]=recvbuffer+recvStarts[i];
    }

    multimerge(mmStarts,recvLengths,numprocs,myData,myDataSize);

    int mysendLength = recvStarts[numprocs-1] + recvLengths[numprocs-1];

    int sendLengths[numprocs];
    int sendStarts[numprocs];

    MPI::COMM_WORLD.Gather(&mysendLength, 1, MPI::INT, sendLengths, 1, MPI::INT, 0);

    if (my_rank == 0) {
        sendStarts[0]=0;
        for(int i=1; i<numprocs; i++) {
            sendStarts[i] = sendStarts[i-1]+sendLengths[i-1];
        }
    }

    int sortedData[myDataSize];
    MPI::COMM_WORLD.Gatherv(myData, mysendLength, MPI::INT, sortedData, sendLengths, sendStarts, MPI::INT, 0);

    if (my_rank == 0) {
        endwtime = MPI::Wtime();
        cout << "wall clock time (seconds) = " << scientific << setprecision(4) << endwtime-startwtime << endl;

        cout << "Data set " << issorted(sortedData,myDataSize) << " sorted:" << endl;
    }

    MPI::Finalize();
    return 0;
}
