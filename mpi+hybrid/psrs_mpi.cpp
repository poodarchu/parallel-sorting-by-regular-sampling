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

int main(int argc, char* argv[]) {
    int my_rank, num_processors;
    double start_t = 0.0, end_t;

    MPI_Init(argc, argv);

    MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int random_seed = 1000;
    int my_data_size = 4000;
    for(int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-DS") == 0) {
            my_data_size = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-SR") == 0) {
            random_seed = atoi(argv[i+1]);
            i++;
        }
    }

    int my_data[my_data_size];
    int my_data_lengths[num_processors];
    int my_data_starts[num_processors];

    int pivot_buffer[num_processors*num_processors];
    int pivot_buffer_sz;

    for(int i = 0; i < num_processors; i++) {
        my_data_lengths[i] = my_data_size/num_processors;
        my_data_starts[i] = i*my_data_size/num_processors;
    }
    my_data_lengths[num_processors-1] += (my_data_size%num_processors);

    if (my_rank == 0) {
        srandom(random_seed);
        for(int index=0; index < my_data_size; index++) {
            my_data[index] = random()% 900;
        }
        start_t = MPI::Wtime();
    }

    if (my_rank == 0) {
        MPI::COMM_WORLD.Scatterv(my_data, my_data_lengths, my_data_starts, MPI::INT, MPI_IN_PLACE, my_data_lengths[my_rank], MPI::INT,0);
    } else {
        MPI::COMM_WORLD.Scatterv(my_data, my_data_lengths, my_data_starts, MPI::INT, my_data, my_data_lengths[my_rank], MPI::INT, 0);
    }

    qsort(my_data,my_data_lengths[my_rank], sizeof(int), compare_ints);

    // All processors collect regular samples from sorted list Consider an offset to the my_data[] index
    for(int index=0; index < num_processors; index++) {
        pivot_buffer[index] = my_data[index*my_data_lengths[my_rank]/num_processors];
    }

    if (my_rank == 0) {
        MPI::COMM_WORLD.Gather(MPI_IN_PLACE,num_processors, MPI::INT, pivot_buffer, num_processors, MPI::INT, 0);
    } else {
        MPI::COMM_WORLD.Gather(pivot_buffer, num_processors, MPI::INT, pivot_buffer, num_processors, MPI::INT, 0);
    }

    if (my_rank == 0) {
        int *starts[num_processors];
        int lengths[num_processors];
        for(int i = 0; i < num_processors; i++) {
            starts[i] = &pivot_buffer[i*num_processors];
            lengths[i] = num_processors;
        }

        int temp_buffer[num_processors*num_processors];  // merged list
        multimerge(starts, lengths, num_processors, temp_buffer, num_processors*num_processors);

        for(int i = 0; i < num_processors-1; i++) {
            pivot_buffer[i] = temp_buffer[(i+1)*num_processors];
        }
    }

    MPI::COMM_WORLD.Bcast(pivot_buffer, num_processors-1, MPI::INT, 0);

    int class_start[num_processors];
    int class_length[num_processors];

    int data_idx = 0;
    for(int class_idx = 0; class_idx < num_processors-1; class_idx++) {
        class_start[class_idx] = data_idx;
        class_length[class_idx]=0;

        while((data_idx< my_data_lengths[my_rank]) && (my_data[data_idx]<=pivot_buffer[class_idx])) {
            class_length[class_idx]++;
            data_idx++;
        }
    }

    class_start[num_processors-1] = data_idx;
    class_length[num_processors-1] = my_data_lengths[my_rank] - data_idx;

    int recv_buffer[my_data_size];
    int recv_lengths[num_processors];
    int recv_starts[num_processors];

    for(int iprocessor = 0; iprocessor < num_processors; iprocessor++) {
        MPI::COMM_WORLD.Gather(&class_length[iprocessor], 1, MPI::INT, recv_lengths, 1, MPI::INT, iprocessor);
        if (my_rank == iprocessor) {
            recv_starts[0]=0;
            for(int i = 1; i < num_processors; i++) {
                recv_starts[i] = recv_starts[i-1] + recv_lengths[i-1];
            }
        }
        MPI::COMM_WORLD.Gatherv(&my_data[class_start[iprocessor]], class_length[iprocessor], MPI::INT, recv_buffer, recv_lengths, recv_starts, MPI::INT, iprocessor);
    }

    int *mm_starts[num_processors];

    for(int i = 0; i < num_processors; i++) {
        mm_starts[i] = recv_buffer+recv_starts[i];
    }

    multimerge(mm_starts, recv_lengths, num_processors, my_data, my_data_size);

    int mysendLength = recv_starts[num_processors-1] + recv_lengths[num_processors-1];

    int sendLengths[num_processors];
    int sendStarts[num_processors];

    MPI::COMM_WORLD.Gather(&mysendLength, 1, MPI::INT, sendLengths, 1, MPI::INT, 0);

    if (my_rank == 0) {
        sendStarts[0] = 0;
        for(int i=1; i < num_processors; i++) {
            sendStarts[i] = sendStarts[i-1]+sendLengths[i-1];
        }
    }

    int sortedData[my_data_size];
    MPI::COMM_WORLD.Gatherv(my_data, mysendLength, MPI::INT, sortedData, sendLengths, sendStarts, MPI::INT, 0);

    if (my_rank == 0) {
        end_t = MPI::Wtime();
        cout << "wall clock time (seconds) = " << scientific << setprecision(4) << end_t-start_t << endl;
        cout << "Data set " << issorted(sortedData,my_data_size) << " sorted:" << endl;
    }

    MPI::Finalize();
    return 0;
}
