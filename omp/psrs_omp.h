
#include "omp.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int lcompare(const void * ptr2num1, const void * ptr2num2);
long long *merge(long long * left, long long * right, int l_end, int r_end);
long long *merge_sort(long long * arr, int size);
void insertion_sort(long long *arr, int n);
void calc_partition_borders(long long array[], int start, int end, int sublist_sizes[], int at, long long pivots[], int first_p, int last_p);
void psrs_sort(long long *a, int n);
void sortll(long long *a, int len);
