#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Erasure code RDP. C++ version
 * @author Roger Song
 *
 */
using namespace std;
class rdp {
    
private:
    int disks;  // data column number
    int stripe_unit_size;  // stripe size
    static const int TOLERENCE = 2; // RDP can protect data against 2 failures
    static const int DATA_LENGTH = 1024; // default data length 
    int pnumRdp;    // a prime number
    int w;
    int allDisks;  // disk num or data unit num
    char  **idata;  // original data
    char  **odata;  // redundant data
    char  **data;   // for decoding
    
public:
	rdp(){
		disks = 4;
		pnumRdp = 257;
		stripe_unit_size = DATA_LENGTH;
		w = pnumRdp - 1;
		allDisks = disks + TOLERENCE;
        
        idata=(char **)malloc(sizeof(char *)*disks);
        for(int i=0;i<disks;i++) {
	        idata[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(idata[i],0,stripe_unit_size);
	    }  
        
        odata=(char **)malloc(sizeof(char *)*TOLERENCE);
        for(int i=0;i<TOLERENCE;i++) {
	        odata[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(odata[i],0,stripe_unit_size);
	    }  

        data=(char **)malloc(sizeof(char *)*allDisks);
        for(int i=0;i<allDisks;i++) {
	        data[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(data[i],0,stripe_unit_size);
	    }  
        
	}
	
	rdp(int disk, int pRdp, int dataLength ){
		pnumRdp = pRdp;
		stripe_unit_size = dataLength;
		w = pnumRdp - 1;
		disks = disk;
		allDisks = disks + TOLERENCE;

        idata=(char **)malloc(sizeof(char *)*disks);
        for(int i=0;i<disks;i++) {
	        idata[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(idata[i],0,stripe_unit_size);
	    }  
        
        odata=(char **)malloc(sizeof(char *)*TOLERENCE);
        for(int i=0;i<TOLERENCE;i++) {
	        odata[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(odata[i],0,stripe_unit_size);
	    }  

        data=(char **)malloc(sizeof(char *)*allDisks);
        for(int i=0;i<allDisks;i++) {
	        data[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(data[i],0,stripe_unit_size);
	    }  
        
	}

    ~rdp(){
        for(int i=0;i<disks;i++) {
	        free(idata[i]);
	        
	    }  
        free(idata);
        
        for(int i=0;i<TOLERENCE;i++) {
	        free(odata[i]);
	    }  
        free(odata);

        for(int i=0;i<allDisks;i++) {
	        free(data[i]);
	    }  
        free(data);
    }
	

	/**
	 *  an easy test case
	 */
	void setData(){
		for(int i = 0; i < disks; i++){
			for(int j = 0; j < stripe_unit_size; j++){
				idata[i][j]=(char) ('a' + i);
			}
		}

        memcpy(odata[0], idata[0], stripe_unit_size* sizeof(char));
        memcpy(odata[1], idata[0], stripe_unit_size* sizeof(char));
	}
	
	string showme(){
		return "RDP";
	}
	

	/**
	 * rdp encoding algorithm
	 */
	void rdp_encoding(){
		
		int off = 0;
        int d = 0;
        int p = 0;
        int diag = 0;
		int packet_size = stripe_unit_size /w;
		
		for (d = 1; d < disks; d++) {
			
			diag = d;

			for (p = 0; p < w; p++) {
				if (diag <= w - 1)
				{
					for (off = 0; off < packet_size; off++) {

						odata[0][p * packet_size + off]^= idata[d][p * packet_size + off];
						odata[1][diag * packet_size + off]^= idata[d][p * packet_size + off];
					}
					diag++;

				}
				else
				{
					
					for (off = 0; off < packet_size; off++) {

						odata[0][p * packet_size + off]^= idata[d][p * packet_size + off];
					}
					diag = 0;
					
				}
				
			}
		}
		for (p = 1; p < w;p++)
		{
			for (off = 0;off < packet_size;off++)
			{
				odata[1][(p - 1) * packet_size + off]^= odata[0][p * packet_size + off];

			}
		}


	}
	

	/**
	 * rdp encoding main function. 
	 * there is a simple testcase in setData func 
	 */

	void encoding(){
		
		setData();
		outputRes();
		rdp_encoding();
	}
	

	/**
	 * rdp decoding main function.
	 * errorNum:[1,2]  one/two:the first/second error disk  rError: r checkout is broken?
	 * @param errorNum  
	 * @param one
	 * @param two
	 * @param rError
	 */
	void decoding(int errorNum, int one, int two, bool rError){
		
		int oneData = one + 2;
		int twoData = two + 2;
		// rdp 
		if(errorNum > 2 || errorNum < 1){
			printf("Error NUM is too larger or smaller! It should be [1,2] \n");
			return;
		}
		if(errorNum == 2 && (one < 0 || one >= disks || two < 0 || one >= disks)){
			printf("Error NUM is 2, but detailed error col numbers are wrong! Thay are should be [0,disks) \n");
			return;
		}

        memcpy(data[0], odata[0], stripe_unit_size* sizeof(char));
        memcpy(data[1], odata[1], stripe_unit_size* sizeof(char));
		
		for(int i = 0; i < disks; i++){
            memcpy(data[i+2], idata[i], stripe_unit_size* sizeof(char));

		}	
		
		for(int i = 0; i < stripe_unit_size; i++){

			data[oneData][i] = 0;
		}
		
		if(errorNum == 1){
			if(rError){
				for(int i = 0; i < stripe_unit_size; i++){
					data[0][i] = 0;
				}
				
				rdp_decoding_d(data, disks, stripe_unit_size, w, oneData);
                memcpy(idata[one], data[oneData], stripe_unit_size* sizeof(char));

			}
			else{
				rdp_decoding_r(data, disks, stripe_unit_size, w, oneData);
                memcpy(idata[one], data[oneData], stripe_unit_size* sizeof(char));

			}
		}
		else{
			for(int i = 0; i < stripe_unit_size; i++){

				data[oneData][i] = 0;
				data[twoData][i] = 0;
			}
			rdp_decoding_rd(data, disks, stripe_unit_size, w, oneData, twoData);
            memcpy(idata[one], data[oneData], stripe_unit_size* sizeof(char));
            memcpy(idata[two], data[twoData], stripe_unit_size* sizeof(char));
		}
		
	}
	

	/**
	 * rdp decoding according r and d checkouts.
	 * x/y:  the first/second error disk in g_data
	 * @param g_data  
	 * @param disks
	 * @param stripe_unit_size
	 * @param w
	 * @param x
	 * @param y
	 */
	void rdp_decoding_rd(char * g_data[], int disks, int stripe_unit_size, int w,int x,int y)
	{
		int packet_size = stripe_unit_size / w;
		int ccount = 0;
        int count = 0;
        int rcount = 0;
		char *rdata, *xdata, *ydata, *row_data, *diag_data;
		int g, gx, gy,coffset,i,j,k,c;
		int row_disk, diag_disk;
		
		rcount=stripe_unit_size;
	
		rdata=g_data[0];
		xdata=g_data[x];
		ydata=g_data[y];
		gx = (x >= 3 ? x - 3 : pnumRdp - 1); 
		gy = (y >= 3 ? y - 3 : pnumRdp - 1);
		row_disk = x;
		diag_disk = y;
		row_data = xdata;
		diag_data = ydata;
		g=gx;
		
		while(1) {
			if(g == pnumRdp - 1) {
				if(gx == (row_disk >= 3 ? row_disk - 3 : row_disk + pnumRdp - 3)) {
					row_disk = y;
					diag_disk = x;
					row_data = ydata;
					diag_data = xdata; 
					g = gy;
					continue;
				}
				else
					break;
			}
			else {
				int  row_index;
				char*  cdata;  
				int diag_count;
				int row_count;
				diag_count=stripe_unit_size;
				row_count=stripe_unit_size;
				row_index = (g - diag_disk + pnumRdp + 2) % pnumRdp;
				coffset = (row_index + diag_disk - 2 + pnumRdp) % pnumRdp * packet_size;
				cdata=g_data[1];
				for(i = row_index * packet_size, j = coffset, k = 0;i < diag_count &&k < packet_size; i++, j++, k++)
				{
					diag_data[i] = cdata[j];
				}
				for(c = 2; c < (disks+2); c++) {					
					if(c == diag_disk)
						continue;
					ccount=stripe_unit_size;

		            cdata = g_data[c];
					coffset = (row_index + diag_disk - c + pnumRdp) 
						% pnumRdp * packet_size;
					for(i = row_index * packet_size, j = coffset, k = 0;i < diag_count && j < ccount &&k < packet_size; i++, j++, k++)
					{

						diag_data[i] ^=  cdata[j];
					}		
				}

	            ccount = rcount;
	            cdata = rdata;
				coffset = (row_index + diag_disk - 1 + pnumRdp) 
				% pnumRdp * packet_size;

				for(i = row_index * packet_size, j = coffset, k = 0; i < diag_count && j < ccount &&k < packet_size; i++, j++, k++)
				{

					diag_data[i] ^=  cdata[j];
				}
				
				//row_parity calculate
				coffset = row_index * packet_size;
				for(c = 2; c < (disks+2); c++) {
					if(c == row_disk)
						continue;

					ccount=stripe_unit_size;

		            cdata = g_data[c];	
					coffset = row_index * packet_size;
					for(i = coffset, k = 0; i < ccount &&k < packet_size; i++, k++)
					{	

						row_data[i] ^= cdata[i];
					}  

				}
				cdata = rdata;
				coffset = row_index * packet_size;
				count = row_count;
				
				for(i = coffset, k = 0; i < count &&  k < packet_size; i++, k++)
				{
					row_data[i] ^= cdata[i];

				}		
				
				//calculate the next g
				g = (row_index + row_disk - 2) % pnumRdp;

			}
		}
		
	}

	
	/**
	 * rdp decoding according r and d checkouts.
	 * x:  the first error disk in g_data
	 * @param g_data  
	 * @param disks
	 * @param stripe_unit_size
	 * @param w
	 * @param x
	 */
	void rdp_decoding_d(char *g_data[],  int disks, int stripe_unit_size, int w, int x)
	{
		int xcount, ccount, count, dcount;
		int packet_size = stripe_unit_size / w;
		int gr,g,gx;
		int row_disk, diag_disk;
		char *row_data, *diag_data, *rdata, *xdata;
		int coffset;
		int i,j,k,c;
		int diag_count;
		int row_count;
		
		
		dcount=stripe_unit_size;
		xcount=stripe_unit_size;
		gr = pnumRdp - 2; 
		gx = (x >= 3 ? x - 3 : pnumRdp - 1);
		row_disk = pnumRdp + 1;
		diag_disk = x;
		rdata=g_data[0];
		xdata=g_data[x];
		row_data=rdata;
		diag_data=xdata;
		g = gr;
		diag_count = xcount;
		row_count = dcount;
		
		
		while(1) {
			if(g == pnumRdp - 1) {
				if(gr == (row_disk >= 3 ? row_disk - 3 : row_disk + pnumRdp -3)) {
					row_disk = x;
					diag_disk = pnumRdp + 1;
					row_data = xdata;
					diag_data =rdata;
					g = gx;
					row_count = diag_count;
					diag_count = dcount;
	
					continue;
				}
				else
					break;
			}
			else {
				int row_index;
				char* cdata;  
				row_index = (g - diag_disk + pnumRdp + 2) % pnumRdp;
			       coffset = (row_index + diag_disk - 2 + pnumRdp) % pnumRdp * packet_size;
				cdata=g_data[1];
				for(i = row_index * packet_size, j = coffset, k = 0; i < diag_count && k < packet_size; i++, j++, k++)
				{
					diag_data[i] = cdata[j];
	
				}
				for(c = 2; c <(disks+2); c++) {
					if((int)c == diag_disk)
						continue;
				
					
					ccount=stripe_unit_size;
					cdata = g_data[c];
					coffset = (row_index + diag_disk - c + pnumRdp) 
						% pnumRdp * packet_size;
	
					for(i = row_index * packet_size, j = coffset, k = 0;i < diag_count && j < ccount && k < packet_size; i++, j++, k++)
					{
						diag_data[i] ^=  cdata[j];
	
					}
						
				}
				if(pnumRdp + 1 != diag_disk)
				{
					ccount=stripe_unit_size;
					cdata = g_data[0];
					coffset = (row_index + diag_disk - 1 + pnumRdp) 
						% pnumRdp * packet_size;
	
					for(i = row_index * packet_size, j = coffset, k = 0;i < diag_count && j < ccount && k < packet_size; i++, j++, k++)
					{
						diag_data[i] ^=  cdata[j];
	
					}
				}
				//row_parity calculate
				for(c = 2; c <(disks+2); c++) {
					if((int)c == row_disk)
						continue;
	
					ccount=stripe_unit_size;
					cdata = g_data[c];				
					coffset = row_index * packet_size;
	
					for(i = coffset, k = 0;i < ccount && k < packet_size; i++, k++)
					{	
						row_data[i] ^= cdata[i];

					}
	
				
				}
				if(row_disk != pnumRdp + 1) {
					
					//diag_disk must be DISK---VDEV_RAIDZ_R 
					count = row_count;
					cdata = diag_data;
					coffset = row_index * packet_size;
	
					for(i = coffset, k = 0; i < count && k < packet_size; i++, k++)
						row_data[i] ^= cdata[i];
	
				}
				//calculate the next g
				g = (row_index + row_disk - 2) % pnumRdp;	
				
			}
		}
	}
	

	/**
	 * rdp decoding according r and d checkouts.
	 * k:  the first error disk in g_data
	 * @param g_data  
	 * @param disks
	 * @param stripe_unit_size
	 * @param w
	 * @param k
	 */
	void rdp_decoding_r(char * g_data[], int disks, int stripe_unit_size, int w, int k)
	
	{
		int off = 0; 
        int p = 0;
		int packet_size = stripe_unit_size / w;
		int i = 0;

    	memcpy(g_data[k],g_data[0],stripe_unit_size*sizeof(char));
		for(i=2;i<(disks+2);i++)
		{
			if(i==k) continue;
			for (p = 0; p < w; p++)
			{	
				for (off = 0; off < packet_size; off++)
		 		{ 
		 		 	g_data[k][p * packet_size + off]^=g_data[i][p * packet_size + off];
				}
			}	
		}
		
	}

	/**
	 * for testing and debug.
	 */
	void outputRes(){
		
		for(int i=0; i < disks; i++)
		{
			printf("idata:%d:  ",i);
			printf(idata[i]);
            printf("\n");
		}
		printf("odata:0:  ");
		printf(odata[0]);
        printf("\n");
		printf("odata:1:  ");
		printf(odata[1]);
        printf("\n");
	}
	
	/**
	 * for testing and debug.
	 */
	void outputOdata(){
		
		printf("The res:");
		printf("odata:0:  ");
		printf(odata[0]);
        printf("\n");
		printf("odata:1:  ");
		printf(odata[1]);
        printf("\n");
	}

	/**
	 * for testing and debug.
	 */
	void outputOrigin(){
		
		printf("After decoding:");
		for(int i=0; i < disks; i++)
		{
			printf("idata:%d:  ",i);
			printf(idata[i]);
            printf("\n");
		}
	}

};

