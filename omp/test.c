#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "psrs_omp.h"

void write_out(long long a[], int size) {
  int i;
  for ( i = 0; i < size; i++ ) {
    printf("%lld\n", a[i]);
  }
}

long long * read_in(char filename[], int * ptr2size) {
  const int max_line = 1024;
  char line[max_line];
  int i;
  FILE * file;
  char * eof;
  long long * a;
  int size;

  file = fopen(filename, "r");
  if ( file == NULL ) {
    fprintf(stderr, "file doesn't exist: %s\n", filename);
    exit(1);
  }

  eof = fgets(line, max_line, file);
  if ( eof == NULL ) {
    fprintf(stderr, "Empty file: %s\n", filename);
    exit(1);
  }
  sscanf(line, "%d", &size);
  a = malloc(sizeof(long long) * size);

  i = 0;
  eof = fgets(line, max_line, file);
  while ( eof != NULL && i < size ) {
    sscanf(line, "%lld", &(a[i]));
    i++;
    eof = fgets(line, max_line, file);
  }

  fclose(file);
  *ptr2size = size;

  return a;
}

long long * copy_array(long long * array, int size)
{
  long long * result = malloc(sizeof(long long) * size);
  int i;

  for ( i = 0; i < size; i++ ) {
    result[i] = array[i];
  }
  return result;
}

int llcompare(const void * ptr2num1, const void * ptr2num2) {
  long long num1 = *((long long*) ptr2num1);
  long long num2 = *((long long*) ptr2num2);

  if ( num1 > num2 )
    return 1;
  else if ( num1 < num2 )
    return -1;
  else
    return 0;
}

void sort(long long a[], int size) {
  qsort(a, size, sizeof(long long), llcompare);
}

long long * gen_random(int size)
{
  long long * result = malloc(sizeof(long long) * size);
  int i;
  struct timeval seedtime;
  int seed;

  gettimeofday(&seedtime, NULL);
  seed = seedtime.tv_usec;
  srandom(seed);

  for ( i = 0; i < size; i++ ) {
    long long upper = random();
    long long lower = random();
    result[i] = (upper << 32) | lower;
  }

  return result;
}

int main(int argc, char ** argv)
{
  long long * array;
  long long * copy;
  long long sort_time;
  int array_size;
  struct timeval start_time;
  struct timeval stop_time;
  int i;

  if ( argc == 3 && !strcmp(argv[1], "-f") ) {
    char * filename = argv[2];
    array = read_in(filename, &array_size);
  }
  else if ( argc == 3 && !strcmp(argv[1], "-r") ) {
    array_size = atoi(argv[2]);
    array = gen_random(array_size);
  }
  
  else {
    fprintf(stderr, "Usage: psrs -f <filename> OR psrs -r <size>\n");
    exit(1);
  }
  DEBUGGING(write_out(array, array_size));

  copy = copy_array(array, array_size);

  gettimeofday(&start_time, NULL);

  psrs_sort(array, array_size);

  gettimeofday(&stop_time, NULL);
  sort_time = (stop_time.tv_sec - start_time.tv_sec) * 1000000L +
    (stop_time.tv_usec - start_time.tv_usec);
  printf("Sorting time: %lld microseconds\n", sort_time);

  DEBUGGING(write_out(array, array_size));

  sort(copy, array_size);

  for ( i = 0; i < array_size; i++ ) {
    if ( array[i] != copy[i] ) {
    }
  }

  return 0;
}
