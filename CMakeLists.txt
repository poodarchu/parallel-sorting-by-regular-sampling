cmake_minimum_required(VERSION 3.7)
project(parallel-sorting-by-regular-sampling)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_COMPILER /usr/local/bin/g++-6)
#set(CMAKE_C_COMPILER /usr/local/bin/gcc-6)

set(CMAKE_C_COMPILER /opt/openmpi/bin/mpicc -fopenmp)
set(CMAKE_CXX_COMPILER /opt/openmpi/bin/mpic++)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

set(SOURCE_FILES omp_psrs.c hybrid_psrs.c mpi_psrs.c psrs_mpi.cpp)

add_executable(parallel-sorting-by-regular-sampling ${SOURCE_FILES})
