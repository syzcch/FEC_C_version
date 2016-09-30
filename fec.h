#ifndef __FEC_H__
#define  __FEC_H__

#include <string.h>


using namespace std;

class fec {
public:	
	virtual void setErrData(int* err) = 0;
	virtual void setData() = 0;
	virtual string showme() = 0;
	virtual void encoding() = 0;
	virtual void decoding() = 0;
	virtual void outputOrigin() = 0;
	virtual void outputData() = 0;
};

#endif