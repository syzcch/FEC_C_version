#include "fec_rdp.h"

/**
 * testing my fec rdp code.
 */
int main() {

	const int NUM = 6;
	int err[NUM];

	printf("starting");
	rdp *rdpItem = new rdp();

	rdpItem->setData();
	rdpItem->encoding();
	rdpItem->outputData();
	
	for(int i=0;i<NUM;i++){
		err[i] = 0;
	}

	err[5]=1;

	rdpItem->setErrData(err);
	
	// testing one error, error disk sequence number is 3, r checkout disk is not broken
//		rdpItem->decoding(1, 3, -1, false);
//		rdpItem->outputOrigin();
	
	// testing one error, error disk sequence number is 1, r checkout disk is  broken
//	rdpItem->decoding(1, 2, -1, false);
	rdpItem->decoding();
	rdpItem->outputOrigin();
	
	// testing 2 error, the error disks sequence number is 0 and 3, r checkout disk is not broken
//	rdpItem->decoding(2, 1, 3, true);
//	rdpItem->outputOrigin();
}
