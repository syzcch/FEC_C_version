#include "fec_rs.h"

/**
 * testing my fec rs code.
 */

int main()
{

    const int NUM = 8;
    printf("starting");
//  rscode *rsItem = new rscode();

    //all original data is 8, checksum data is 3, data stripe length is 1024
    rscode *rsItem = new rscode(11,3,1024);
    int *err = new int[NUM];

    // 1 means fault data
    for(int i=0; i<NUM; i++)
    {
        err[i] = 0;
    }

    rsItem->setData();
    rsItem->encoding();
    rsItem->outputData();


    // testing 3 errors, error disk sequence number is 0,1,3
    err[0]=1;
    err[1]=1;
    err[3]=1;
    rsItem->setErrData(err);
    rsItem->decoding();
    rsItem->outputOrigin();

}