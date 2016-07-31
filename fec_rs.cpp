#include "fec_rs.h"

/**
 * testing my fec rs code.
 */

int main() {

    const int NUM = 8;
	printf("starting");
//	rscode *rsItem = new rscode();

    //all data is 8, checksum data is 3, data stripe length is 1024
    rscode *rsItem = new rscode(8,3,1024);
    int *err = new int[NUM];

    // 0 means fault data
    for(int i=0;i<NUM;i++){
        err[i] = 1;
    }

    rsItem->setData();
	rsItem->encoding_rs();
	rsItem->outputOdata();
	
	
	// testing 3 errors, error disk sequence number is 0,1,3
    err[1]=0;
    err[3]=0;
    rsItem->setErrData(err);
    rsItem->decoding_rs();
	rsItem->outputOrigin();
	

}