//
// Created by Poodar Chu on 08/07/2017.
//

#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>

using namespace std;

int compare_ints(const void *a, const void *b);
string issorted(int xxx[], int xxxStart, int xxxLength);
string issorted(int xxx[], int xxxLength);
void dumpArray(int myid, string arrayName, int array[], int start, int length);
void dumpArray(int myid, string arrayName, int array[], int length);

#endif
