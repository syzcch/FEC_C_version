#include "fec_star.h"

/**
 * testing my fec rdp code.
 */
int main() {

    printf("starting");

    const int NUM = 9;
    int *err = new int[NUM];
//    star *starItem = new star();
    star *starItem = new star(6,257,1024);
    
    starItem->setData();
    starItem->encoding();
    starItem->outputData();

    // 1 means fault data
    for(int i=0;i<NUM;i++){
        err[i] = 0;
    }

//    err[0]=1;
 //   err[1]=1;
    err[2]=1;
//    err[3]=1;

    starItem->setErrData(err);
    starItem->decoding();
    starItem->outputOrigin();
    
}
