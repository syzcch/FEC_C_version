//#include "fec.h"
#include "fec_rdp.h"
#include "fec_rs.h"
#include "fec_star.h"

/**
 * testing all my fec code.
 */
int main() {

    printf("starting");

    // the quantity of data columns
    const int NUM = 6;
    int *err = new int[NUM];

//    fec *fecCode = new rscode(10,3,1024);
//    fec *fecCode = new star(6,257,1024);
    fec *fecCode = new rdp();

    
    fecCode->setData();
    fecCode->encoding();
    fecCode->outputData();

    // 1 means fault data
    for(int i=0;i<NUM;i++){
        err[i] = 0;
    }

    err[0]=1;
 //   err[1]=1;
 //   err[2]=1;
    err[3]=1;

    fecCode->setErrData(err);
    fecCode->decoding();
    fecCode->outputOrigin();
    
}

