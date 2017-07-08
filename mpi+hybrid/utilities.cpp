//
// Created by Poodar Chu on 08/07/2017.
//

#include "utilities.h"

string issorted(int xxx[], int xxxStart, int xxxLength) {
    for(int i=xxxStart; i<xxxStart+xxxLength-1; i++) {
        if (xxx[i]>xxx[i+1])
            return "is not";
    }
    return "is";
}

string issorted(int xxx[], int xxxLength) {
    return issorted(xxx,0,xxxLength);
}

int compare_ints(const void *a, const void *b) {
    int myint1 = *reinterpret_cast<const int *>(a);
    int myint2 = *reinterpret_cast<const int *>(b);
    if (myint1<myint2) return -1;
    if (myint1>myint2) return 1;
    return 0;
}

void dumpArray(int myid, string arrayName, int array[], int start, int length) {
    for(int i=start;i<start+length;i++) {
        cout << myid << ": " << arrayName << "[" << i << "] = " << array[i] << endl;
    }
}

void dumpArray(int myid, string arrayName, int array[], int length) {
    dumpArray(myid, arrayName, array, 0, length);
}
