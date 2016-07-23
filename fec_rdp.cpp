#include "fec_rdp.h"

/**
 * testing my fec rdp code.
 */
int main() {

	printf("starting");
	rdp *rdpItem = new rdp();

	rdpItem->encoding();
	rdpItem->outputOdata();
	
	// testing one error, error disk sequence number is 3, r checkout disk is not broken
//		rdpItem->decoding(1, 3, -1, false);
//		rdpItem->outputOrigin();
	
	// testing one error, error disk sequence number is 1, r checkout disk is  broken
		rdpItem->decoding(1, 2, -1, false);
		rdpItem->outputOrigin();
	
	// testing 2 error, the error disks sequence number is 0 and 3, r checkout disk is not broken
//	rdpItem->decoding(2, 1, 3, true);
//	rdpItem->outputOrigin();
}