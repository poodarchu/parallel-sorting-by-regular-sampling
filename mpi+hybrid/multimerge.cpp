//
// Created by Poodar Chu on 08/07/2017.
//

#include "multimerge.h"

struct mmdata {
    int stindex;
    int index;
    int stvalue;
    mmdata(int st=0, int id=0, int stv = 0):stindex(st),index(id),stvalue(stv){}
};

bool operator<( const mmdata & One, const mmdata & Two) {
    return One.stvalue > Two.stvalue;
}

int multimerge(int * starts[], const int lengths[], const int Number, int newArray[], const int newArrayLength) {
    priority_queue< mmdata> priorities;

    for(int i=0; i<Number;i++) {
        if (lengths[i]>0) {
            priorities.push(mmdata(i,0,starts[i][0]));
        }
    }

    int newArrayindex = 0;
    while (!priorities.empty() && (newArrayindex<newArrayLength)) {
        mmdata xxx = priorities.top();
        priorities.pop();

        newArray[newArrayindex++] = starts[xxx.stindex][xxx.index];

        if ( lengths[xxx.stindex]>(xxx.index+1)) {
            priorities.push(mmdata(xxx.stindex, xxx.index+1, starts[xxx.stindex][xxx.index+1]));
        }
    }
    return newArrayindex;
}
