#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Erasure code STAR. C++ version
 * @author Roger Song
 *
 */
using namespace std;
class star{

private:
    int  check_data_size;
    int block_size;
    int p;
    int block_nbr;/*block_nbr = p - 1*/
    int **check_data;
    int *restarts;
    int data_disk_nbr;
    int allNum;
    int stripe_unit_size;  // stripe size
    static const int TOLERENCE = 3; // STAR can protect data against 2 failures
    static const int DATA_LENGTH = 1024; // default data length 
    static const int PRIME = 257; // a prime number
public:
    star()
    {
        p = PRIME;
        data_disk_nbr = 4;
        stripe_unit_size = DATA_LENGTH;  //1024
        check_data_size = stripe_unit_size * sizeof(char) / sizeof(int);   //256
        block_nbr = p - 1;   //256
        block_size = check_data_size / block_nbr;   //1
        allNum = data_disk_nbr + TOLERENCE;  //7

        check_data = (int **)malloc(sizeof(int *) * (allNum));
        for(int i = 0; i < allNum; i++)
        {
            check_data[i] = (int *) malloc(sizeof(int )*check_data_size);
            memset(check_data[i],0,stripe_unit_size);
        }

        restarts = new int[allNum];
        for(int i = 0; i < allNum; i++)
        {
            restarts[i] = 0;
        }        
    }
    
    star(int disk, int prime, int dataLength )
    {
        p = prime;
        data_disk_nbr = disk;
        stripe_unit_size = dataLength;
        check_data_size = stripe_unit_size * sizeof(char) / sizeof(int);
        block_nbr = p - 1;
        block_size = check_data_size / block_nbr;
        allNum = data_disk_nbr + TOLERENCE;

        check_data = (int **)malloc(sizeof(int *) * (allNum));
        for(int i = 0; i < allNum; i++)
        {
            check_data[i] = (int *) malloc(sizeof(int )*check_data_size);
            memset(check_data[i],0,stripe_unit_size);
        }

        restarts = new int[allNum];
        for(int i = 0; i < allNum; i++)
        {
            restarts[i] = 0;
        }
    }

    ~star()
    {
        for(int i = 0; i < allNum; i++) {
	        free(check_data[i]);
	        
	    }  
        free(check_data);
    }

    /**
	 *  an easy test case
	 */
	void setData(){
		for(int i = 0; i < data_disk_nbr; i++){
			for(int j = 0; j < check_data_size; j++){
				check_data[i][j]= 0 + i;
			}
		}
	}

    // 1 means error, default value is 0
    void setErrData(int *err)
    {
        for(int i = 0; i < allNum; i++ )
        {
            restarts[i] = err[i];
        }
    }

    /**
	 * for testing and debug.
	 */
	void outputData()
	{
		
		printf("The res:");
        for(int i = 0; i < allNum; i++)
        {
            for(int j = 0; j < check_data_size; j++)
            {
                printf("%d ",check_data[i][j]);
            }
            
            printf("\n");
        }
	}

    /**
	 * for testing and debug.
	 */
	void outputOrigin(){
		
		printf("After decoding:");
		for(int i=0; i < data_disk_nbr; i++)
		{
			printf("data:%d:  ",i);
			for(int j = 0; j < check_data_size; j++)
            {
                printf("%d ",check_data[i][j]);
            }
            printf("\n");
		}
	}

    /**
	 *  entry function for encoding
	 */
    void STAR_encoding()
    {
        if( check_data_size % block_nbr != 0)
    	{
    		printf(" Cannot  striping. wrong DATA_LENGTH!\n");
    		exit(1);
    	}
        
        STAR_encoding_row  ();
    	STAR_encoding_diag1();
    	STAR_encoding_diag2();
    }

    /*  computing checksum in every row,*check_data[p]  */
    void STAR_encoding_row()
    {
    	int i,j;
    	for( i = 0; i < check_data_size; i++) 
    	{
    		for( j = 0; j < data_disk_nbr; j++) 
    		{
    			check_data[data_disk_nbr][i] ^= check_data[j][i] ;
    		}

    	}
    }

    /* diagonal line checksum, slope is 1,*check_data[p+1] */
    void STAR_encoding_diag1()
    {
    	int i,j,stripe,k;
    	int **tmp;

    	tmp =  (int**)malloc((block_nbr+1) * sizeof(int*));
    	for( i = 0; i < block_nbr + 1; i++)
    	{
    		tmp[i] = (int*)malloc(sizeof(int) * block_size);
    		memset(tmp[i], 0 ,block_size*sizeof(int));
    	}
    	for( stripe = 0; stripe < block_nbr+1; stripe++)
    	{
    		for( i = 0; i < data_disk_nbr;i++)
    		{
    			for( j = 0; j < block_size; j++)
    			{
    				k = (stripe - i + p) % p;
    				if( k < block_nbr)
    				{
    					tmp[stripe][j] ^= check_data[i][(stripe - i + p)%p * block_size + j];

    				}
    			}

    		}

    	}
    	/*after store diagonal line checksum in tmp[block_nbr]*/
    	/*we need using diagonal line checksum and s to do xor compute */
    	for( i = 0; i<block_nbr; i++)
    	{
    		for( j = 0; j<block_size; j++)
    		{
    			tmp[i][j] = tmp[i][j] ^ tmp[block_nbr][j];

    		}
    	}

    	for( i = 0; i < block_nbr; i++)
        {
    		memmove(check_data[data_disk_nbr + 1] + (i * block_size), tmp[i], block_size * sizeof(int));
    	}
        
        for(int i = 0; i < block_nbr+1; i++) {
	        free(tmp[i]);
	        
	    }  
    	free(tmp);
    }

    /* diagonal line checksum, slope -1,*check_data[p+2] */
    void STAR_encoding_diag2()
    {
    	int i,j,stripe,k;
    	int *tmp;
    	tmp = (int*)malloc(sizeof(int) * (p * block_size));
    	memset(tmp, 0, p*block_size*sizeof(int));

    	for(stripe = 0; stripe < block_nbr+1; stripe++)
    	{
    		for(i = 0; i < data_disk_nbr; i++)
    		{
    			for(j = 0; j<block_size; j++)
    			{
    				k = (stripe + i + p) % p;
    				if( k < block_nbr)
    					tmp[stripe * block_size + j] ^= check_data[i][ k * block_size + j];
    			}
    		}
    	}
    	for( i = 0; i< block_nbr; i++)
    	{
    		for( j = 0; j< block_size; j++)
    		{
    			tmp[ i * block_size + j] ^= tmp[block_nbr * block_size + j];
    		}
    	}

    	memmove( check_data[data_disk_nbr + 2], tmp, check_data_size * sizeof(int));

    	free(tmp);
    }

    /**
	 *  entry function for decoding
	 */
    void STAR_decoding()
    {
    	int i,j,k,m,stripe;
    	int rs_nbr = 0;      /*rs_nbr means error data number*/
    	int rs_data_nbr = 0; /*rs_data_nbr means how many error data in original data*/
    	int rs_check_nbr = 0;/*rs_check_nbr means how many error data in redundancy data*/
    	int rs_disk1 = -1;
    	int rs_disk2 = -1;
    	int rs_disk3 = -1;

        if( check_data_size % block_nbr != 0)
    	{
    		printf(" Cannot  striping. wrong DATA_LENGTH!\n");
    		exit(1);
    	}

        for(i = 0; i < data_disk_nbr+ 2; i++)
    	{
    		if(restarts[i] == 1)
    		{	
    			rs_disk1 = i;
    			break;
    		}
    	}
    	if( rs_disk1 != -1)
    	{
    		for( i = rs_disk1 + 1; i < data_disk_nbr + 2; i++)
    		{
    			if(restarts[i] == 1)
    			{
    				rs_disk2 = i;
    				break;
    			}
    		}
    	}
    	if(rs_disk2 != -1)
    	{
    		for( i = rs_disk2 + 1; i < data_disk_nbr + 2; i++)
    		{
    			if(restarts[i] == 1)
    			{
    				rs_disk3 = i;
    				break;
    			}
    		}
    	}

        if(rs_disk1 != -1)memset(check_data[rs_disk1], 0 ,check_data_size*sizeof(int));
    	if(rs_disk2 != -1)memset(check_data[rs_disk2], 0 ,check_data_size*sizeof(int));
    	if(rs_disk3 != -1)memset(check_data[rs_disk3], 0 ,check_data_size*sizeof(int));


//    	printf("rs_disks : %d %d %d\n", rs_disk1,rs_disk2,rs_disk3);
        
    	for( i = 0; i <= data_disk_nbr + 2; i++)
    		rs_nbr += restarts[i]; 

        if( TOLERENCE < rs_nbr)
    	{
    		printf(" Too many error data!\n");
    		exit(1);
    	}
        
        //printf("rs_nbr = %d\n",rs_nbr);
    	for( i = 0; i < data_disk_nbr; i++)
    		rs_data_nbr += restarts[i];
        
    	rs_check_nbr = rs_nbr - rs_data_nbr;

        if(rs_data_nbr == 0)
    	{
    		if(restarts[data_disk_nbr] == 1)
    			STAR_encoding_row();
    		if(restarts[data_disk_nbr + 1] == 1)
    			STAR_encoding_diag1();
    		if(restarts[data_disk_nbr + 2] == 1)
    			STAR_encoding_diag2();
    	}

        if(rs_data_nbr == 1)
    	{
    		if( rs_check_nbr <= 1)
    		{
    			Evenodd_decoding(restarts);
    			if(restarts[data_disk_nbr + 2] == 1)
    				STAR_encoding_diag2();
    		}
    		if(rs_check_nbr == 2)
    		{
    			if( restarts[data_disk_nbr] == 0)/*no error in row checksum*/
    			{
    				for( i = 0; i < check_data_size; i++)
    				{
    					for( j = 0; j <= data_disk_nbr; j++)
    					{
    						if( j != rs_disk1)
    							check_data[rs_disk1][i] ^= check_data[j][i];
    					}
    				}
    				STAR_encoding_diag1();
    				STAR_encoding_diag2();
    			}
    			if(restarts[data_disk_nbr] == 1)/*row checksum is broken*/
    			{
    				if(restarts[data_disk_nbr + 2] == 1)
    				{
    					Evenodd_decoding(restarts);
    					STAR_encoding_diag2();
    				}
    				if(restarts[data_disk_nbr + 1] == 1)
    				{
    					Evenodd_decoding_1(rs_disk1, rs_disk2);
    					STAR_encoding_diag1();
    				}
    			}
    		}
    	}

        if(rs_data_nbr == 2)
    	{
    		if(rs_check_nbr == 0)
    		{
    			Evenodd_decoding(restarts);
    		}
    		else /*rs_check_nbr == 1*/
    			if(restarts[data_disk_nbr] == 1)
    			{
    				int *tmp_for_s1s2;
    				tmp_for_s1s2 = (int *)malloc(sizeof(int) * block_size);
    				memset( tmp_for_s1s2, 0, sizeof(int)  * block_size);
    				for( i = 0; i < block_nbr; i++)/*s1 xor s2*/
    				{
    					for( j = 0; j < block_size; j++)
    					{
    						tmp_for_s1s2[j] ^= check_data[data_disk_nbr + 1][i * block_size + j];
    						tmp_for_s1s2[j] ^= check_data[data_disk_nbr + 2][i * block_size + j];
    					}
    				}

    				int **tmp;// storing s
    				tmp = (int **)malloc( sizeof(int *) * 3);
    				for( i = 0; i < 3; i++)
    				{
    					tmp[i] = (int *)malloc(sizeof(int) * p * block_size);
    					memset( tmp[i], 0, sizeof(int) * p * block_size);
    				}

    				for( i = 0; i < check_data_size; i++)
    				{
    					for ( j = 0; j <= data_disk_nbr; j++)
    					{
    						tmp[0][i] ^= check_data[j][i] ;
    					}
    				}
    				for( stripe = 0; stripe < block_nbr + 1; stripe++)
    				{
    					for( i = 0; i < data_disk_nbr; i++)
    					{
    						for( j = 0; j < block_size; j++)
    						{
    							k = (stripe - i + p) % p;
    							if( k < block_nbr)
    							{
    								tmp[1][stripe * block_size + j] ^= check_data[i][k * block_size + j];
    							}
    						}
    					}
    				}
    				for(  i = 0; i < check_data_size; i++)
    				{
    					tmp[1][i] ^= check_data[data_disk_nbr + 1][i];
    				}


    				for( stripe = 0; stripe < block_nbr + 1; stripe++)
    				{
    					for( i = 0; i < data_disk_nbr; i++)
    					{
    						for( j = 0; j < block_size ; j++)
    						{
    							 k = (stripe + i + p) % p;
    							 if( k < block_nbr)
    							 {
    								 tmp[2][stripe * block_size + j] ^= check_data[i][k * block_size + j];
    							 }
    						}
    					}
    				}
    				for( i = 0 ;i < check_data_size; i++)
    				{
    					tmp[2][i] ^= check_data[data_disk_nbr + 2][i];
    				}
                    
    				/*finish computing all s~~ ,storing them in **tmp */
    				int **tmp_for_xor;
    				tmp_for_xor = (int **)malloc( sizeof(int *) * p);
    				for( i = 0; i < p; i++)
    				{
    					tmp_for_xor[i] = (int *)malloc(sizeof(int) * block_size);
    					memset( tmp_for_xor[i], 0, sizeof(int) * block_size);
    				}

    				for( i = 0; i < block_nbr + 1; i++)
    				{
    					for( j = 0; j < block_size; j ++)
    					{
    						tmp_for_xor[i][j] = tmp[0][i * block_size + j] ^ tmp[0][((rs_disk2 - rs_disk1 + i) % p) * block_size + j]
    						                    ^ tmp[1][((rs_disk2 + p + i) % p) * block_size + j] ^ tmp[2][((p - rs_disk1 + i) % p) * block_size + j]
    											^ tmp_for_s1s2[j];
    					}
    				}

    				/*    *tmp[0] = c[p][0] xor c[s][rs_disk2 - rs_disk1] 		  
    					  *tmp[i] = c[p][i] xor c[s][(rs_disk2 - rs_disk1 + i)%p]
    				  ...................................
    				*/
    				/*c[p][k] xor c[p][p-1] = tmp_for_xor[k], and [rs_disk2][p-1] is zero*/
    				k = p - 1 - ( rs_disk2 - rs_disk1);
    				for( i = 0; i< block_size; i++)
    				{
    					check_data[data_disk_nbr][k * block_size + i] = tmp_for_xor[k][i];
    				}
    	
    				m = block_nbr - 1;
    				while(m)
    				{
    					i = (k + rs_disk1 - rs_disk2 + p) % p;
    				
    					for( j = 0; j < block_size; j++)
    					{
    						check_data[data_disk_nbr][i * block_size + j] = tmp_for_xor[i][j] ^ check_data[data_disk_nbr][k * block_size + j];
    					}
    					k = i;
    					m--;
    				}
    				restarts[data_disk_nbr] = 0;/*restore p*/
    				free(tmp);
    				free(tmp_for_s1s2);
    				free(tmp_for_xor);

    				Evenodd_decoding( restarts);
    			}

    			if(restarts[data_disk_nbr + 1] == 1)
    			{
    				Evenodd_decoding_1(rs_disk1, rs_disk2);
    				STAR_encoding_diag1();
    			}
    			if(restarts[data_disk_nbr + 2] == 1)
    			{
    				Evenodd_decoding(restarts);
    				STAR_encoding_diag2();
    			}
    	}

        /*3 error data*/
        if(rs_data_nbr == 3)
    	{
    		int r,s,t,u,v;

    		int **tmp;/*store s~~*/
    		tmp = (int **)malloc( sizeof(int *) * 3);
    		for( i = 0; i < 3; i++)
    		{
    			tmp[i] = (int *)malloc(sizeof(int) * p * block_size);
    			memset( tmp[i], 0, sizeof(int) * p * block_size);
    		}
    		int *tmp_for_s1;
    		tmp_for_s1 = (int *)malloc(sizeof(int) * block_size);
    		memset( tmp_for_s1, 0, sizeof(int) * block_size);

    		int *tmp_for_s2;
    		tmp_for_s2 = (int *)malloc(sizeof(int) * block_size);
    		memset( tmp_for_s2, 0, sizeof(int) * block_size);

    		int **tmp_for_xor;
    		tmp_for_xor = (int **)malloc( sizeof(int *) * p);
    		for( i = 0; i < p; i++)
    		{
    			tmp_for_xor[i] = (int *)malloc(sizeof(int) * block_size);
    			memset( tmp_for_xor[i], 0, sizeof(int) * block_size);
    		}
    		

    		/*computing s~~*/
    		for( i = 0; i < block_nbr; i++)
    		{
    			for( j = 0; j < block_size; j++)
    			{
    				tmp_for_s1[j] ^= check_data[data_disk_nbr][i * block_size + j];
    				tmp_for_s1[j] ^= check_data[data_disk_nbr + 1][i * block_size + j];

    				tmp_for_s2[j] ^= check_data[data_disk_nbr][i * block_size + j];
    				tmp_for_s2[j] ^= check_data[data_disk_nbr + 2][i * block_size + j];
    			}
    		}

    		
    		for( i = 0; i < check_data_size; i++)
    		{
    			for ( j = 0; j <= data_disk_nbr; j++)
    			{
    				tmp[0][i] ^= check_data[j][i] ;
    			}
    		}

    		for( stripe = 0; stripe < block_nbr + 1; stripe++)
    		{
    			for( i = 0; i < data_disk_nbr; i++)
    			{
    				for( j = 0; j < block_size; j++)
    				{
    					k = (stripe - i + p) % p;
    					if( k < block_nbr)
    					{
    						tmp[1][stripe * block_size + j] ^= check_data[i][k * block_size + j];
    					}
    				}
    			}
    		}
    		for(  i = 0; i < block_nbr + 1; i++)
    		{
    			for(j = 0; j < block_size; j++)
    			{
    				if( i < block_nbr)
    					tmp[1][i * block_size + j] ^= (check_data[data_disk_nbr + 1][i * block_size + j] ^ tmp_for_s1[j]);
    				else tmp[1][i * block_size + j] ^= tmp_for_s1[j];
    			}
    		}


    		for( stripe = 0; stripe < block_nbr + 1; stripe++)
    		{
    			for( i = 0; i < data_disk_nbr; i++)
    			{
    				for( j = 0; j < block_size ; j++)
    				{
    					 k = (stripe + i + p) % p;
    					 if( k < block_nbr)
    					 {
    						 tmp[2][stripe * block_size + j] ^= check_data[i][k * block_size + j];
    					 }
    				}
    			}
    		}

    		for( i = 0 ;i < block_nbr + 1; i++)
    		{
    			for(j = 0; j < block_size; j++)
    			{
    				if( i < block_nbr)
    					tmp[2][i * block_size + j] ^= (check_data[data_disk_nbr + 2][i * block_size + j] ^ tmp_for_s2[j]);
    				else tmp[2][i * block_size + j] ^= tmp_for_s2[j];
    			}
    		}
            
    		/*finish computing all s~~ ,storing them in **tmp */

    		r = rs_disk1;
    		s = rs_disk2;
    		t = rs_disk3;
    		u = s - r;
    		v = t - s;

            /*deal with 3 error data with symmetry*/
    		if( u == v)
    		{
    			for( i = 0; i < block_nbr + 1; i++)
    			{
    				for( j = 0; j < block_size; j ++)
    				{
    					tmp_for_xor[i][j] = tmp[0][i * block_size + j] ^ tmp[0][((t - r + i) % p) * block_size + j]
    					                    ^ tmp[1][((t + p + i) % p) * block_size + j] ^ tmp[2][((p - r + i) % p) * block_size + j];
    				}
    			}
    			/*    *tmp_for_xor[0] = c[s][0] xor c[s][t-r] 
    				  *tmp_for_xor[1] = c[s][1] xor c[s][(t-r+1)%p]
    				  *tmp_for_xor[i] = c[s][i] xor c[s][(t-r+i)%p]
    				  ...................................*/
    			/*c[s][k] xor c[s][p-1] = tmp_for_xor[k],and c[s][p-1] is zero*/
    			k = p - 1 -( t - r );
    			for( i = 0; i< block_size; i++)
    			{
    				check_data[s][k * block_size + i] = tmp_for_xor[k][i];
    			}

    			m = block_nbr - 1;
    			while(m)
    			{
    				i = (r - t + k + p) % p;
    				
    				for( j = 0; j < block_size; j++)
    				{
    					check_data[s][i * block_size + j] = tmp_for_xor[i][j] ^ check_data[s][k * block_size + j];
    				}
    				k = i;
    				m--;
    			}
    		}

            /*asymmetry*/
    		else if( u != v)
    		{	
    			int d;/*d means cross*/
    			int **flag;
    					
    			flag = (int**)malloc(3*sizeof(int*));
    			for(i=0;i<3;i++)
    			{
    				flag[i] = (int*)malloc(sizeof(int) * p);
    				memset(flag[i],0, p*sizeof(int));
    			}
    	
    			for(d = 0;d <= p; d++)
    			{
    				if( (u + v * d) % p == 0)
    					break;
    			}
    			
    			for(i = 0; i< d ; i++ )
    			{
    				/*slope -1*/
    				flag[0][(0 + i * v) % p] ++;
    				flag[1][(s - r + i * v) % p]++;
    				flag[2][(t - r + i * v) % p]++;
    	
    				/*slope 1*/
    				flag[2][(0 + i * v) % p]++;
    				flag[1][(t-s+i * v) % p]++;
    				flag[0][(t-r+i * v) % p]++;
    				
    			}

    			int *count;
    			count = (int*)malloc(sizeof(int) * p);
    			memset(count, 0, sizeof(int)*p);

    			for(i = 0;i < p; i++)/*find how many 1 in line i*/
    			{
    				for(j = 0; j<3; j++)
    				{
    					if(flag[j][i] == 1)
    						count[i]++;					
    				}
    			}

    			for( m = 0; m < block_nbr + 1; m++)/*a xor a */
    			{
    				for( i = 0; i < p ; i++)
    				{
    					if(count[i] == 2 || count[i] == 3)
    					{
    						for( j = 0; j < block_size; j++)
    							tmp_for_xor[m /** v*/][j] ^= tmp[0][((i + m /** v*/) % p) * block_size + j];
    					}
    				}
    				for( i = 0; i < d; i++)
    				{
    					for( j = 0; j < block_size; j++)
    					{
    						tmp_for_xor[m][j] ^= tmp[1][ ((t + p + 0 + i * v + m) % p) * block_size + j];
    						tmp_for_xor[m][j] ^= tmp[2][ ((p - r + 0 + i * v + m) % p) * block_size + j];
    					}
    				}
    			}
    			/*now,*tmp_for_xor[0] = c[s][u] xor c[s][p-u]  
    			       *tmp_for_xor[i] = c[s][(u + i)%p] xor c[s][ ( p-u+i)%p ]
    				   ...............................................*/
    			i = u - 1;
    			k = (u + i) % p;/*c[s][k] xor c[s][p-1] = tmp_for_xor[k],and c[s][p-1] is zero*/
    			for( j = 0; j< block_size; j++)
    			{
    				check_data[s][k * block_size + j] = tmp_for_xor[i][j];
    			}

    			m = block_nbr - 1;
    			while(m)
    			{
    				i = (k + u) % p;
    				
    				for( j = 0; j < block_size; j++)
    				{
    					check_data[s][((u + i) % p) * block_size + j] = tmp_for_xor[i][j] ^ check_data[s][k * block_size + j];
    				}
    				k = (u + i) % p;
    				m--;
    			}
                
    		/* is count[i]==3,then s = s XOR s[0][i]*/
    		/*next, the value of count[u] & count[p-u] is equal 1 or 2*/
    		/*
    		if((count[u] == 1) && (count[p-u]==1))
    		{
    			then a[s][u] XOR a[s][p-u] = s XOR s[1][t] XOR s[2][(p-r)%p] XOR s[1][(t+v)%p] XOR ......
    		}
    		else if(count[u] ==2)
    		{
    		  then using s[0][u]and s[0][p-u]
    		}
    		*/
    		}

    		restarts[s] = 0;/*data s have been restore*/
    		free(tmp);
    		free(tmp_for_s1);
    		free(tmp_for_s2);
    		free(tmp_for_xor);

    		Evenodd_decoding(restarts);
        }
    }

    /*checksum data:row checksum and slope -1 diagonal line
     * ;like evenodd
     */
    void Evenodd_decoding_1(int rs_disk1, int rs_disk2)
    {

        /*2 situations:1,one data error + row data error
    					  2,two data error*/
    	int i,j,stripe,k;
    	int *tmp;
    	tmp = (int*)malloc(sizeof(int) * (p * block_size));
    	memset(tmp, 0, p*block_size*sizeof(int));

        int *tmp_for_s;
    	tmp_for_s = (int *)malloc(sizeof(int) * block_size);
    	memset(tmp_for_s, 0, block_size *sizeof(int));

        /*one data error + row data error*/
    	if(rs_disk1 < data_disk_nbr && rs_disk2 == data_disk_nbr)
    	{
			for(stripe = 0; stripe < block_nbr+1; stripe++)
			{
				for(i = 0; i < data_disk_nbr; i++)
				{
					for(j = 0; j<block_size; j++)
					{
						k = (stripe + i + p) % p;
						if( k < block_nbr)
							tmp[stripe * block_size + j] ^= check_data[i][ k * block_size + j];
					}
				}
			}
		
			/*find out s*/
			stripe = ( p - rs_disk1 - 1) % p;
			
			for( i = 0; i < block_size; i++)
			{
				if(stripe ==  p - 1)
					tmp_for_s[i] = tmp[stripe * block_size + i];
				else
					tmp_for_s[i] = tmp[stripe * block_size + i] ^ check_data[data_disk_nbr + 2][stripe * block_size + i];
			}

			for( i = 0; i < block_nbr; i++)
			{
				for( j = 0; j < block_size; j++)
				{
					tmp[i * block_size + j] ^= (tmp_for_s[j] ^ check_data[data_disk_nbr + 2][ i * block_size + j]);
				}
			}
			for( i = 0; i < block_size; i++)
				tmp[block_nbr * block_size + i]^=tmp_for_s[i];
			/*now all restored data is store in tmp.*/
				
			for( i = 0; i < p; i++)
			{
				j = (i + p + rs_disk1) % p;
				if( j < p - 1)
				{
					memmove( check_data[rs_disk1] + j * block_size, tmp + i * block_size, block_size * sizeof(int));/*********************/
																													 /*********************/
				}
			}
			STAR_encoding_row();
	    }

    	if(rs_disk1 < data_disk_nbr && rs_disk2 < data_disk_nbr)
    	{
    		for( i = 0; i < block_nbr; i++)
    		{	
    			for( j = 0; j < block_size; j++)
    			{
    				tmp_for_s[j] ^= check_data[data_disk_nbr][i * block_size + j];
    				tmp_for_s[j] ^= check_data[data_disk_nbr + 2][i * block_size + j];
    			}
    		}

    	/*	for( i = 0; i < block_nbr; i++)
    		{
    			for( j = 0; j < block_size; j++)
    			{
    				check_data[p + 1][i * block_size + j] ^= tmp_for_s[j];
    			}
    		}
        */

    		for(stripe = 0; stripe < block_nbr+1; stripe++)
    		{
    			for(i = 0; i < data_disk_nbr; i++)
    			{
    				for(j = 0; j < block_size; j++)
    				{
    					k = (stripe + i + p) % p;
    					if( k < block_nbr)
    						tmp[stripe * block_size + j] ^= check_data[i][ k * block_size + j];
    				}
    			}
    		}
    		for( i = 0; i< block_nbr ; i++)
    		{
    			for( j = 0; j< block_size; j++)
    			{
    				if( i < block_nbr)
    					tmp[ i * block_size + j] ^= (check_data[data_disk_nbr + 2][i * block_size + j] ^ tmp_for_s[j]);
    				//else tmp[ i * block_size + j] ^= tmp_for_s[j];
    			}
    		}
    		for( j = 0; j< block_size; j++)
    			tmp[ block_nbr * block_size + j] ^= tmp_for_s[j];
    		/*s store in tmp*/

    		stripe = ( p - rs_disk1 - 1) % p;
            /*apply tmp_for_s to store temporary data*/
    		memmove( tmp_for_s, tmp + stripe * block_size, block_size* sizeof(int));

    		while(1)
    		{
    			k = (stripe + rs_disk2 + p) % p;/*rs_disk2*/
    			if( k == block_nbr)
    				break;
    			memmove( check_data[rs_disk2] + k * block_size, tmp_for_s, block_size * sizeof(int));

    			for( i = 0; i < block_size; i++)
    			{
    				for( j = 0 ; j <= data_disk_nbr ; j++)
    					if( j != rs_disk1)
    						check_data[rs_disk1][k * block_size + i] ^= check_data[j][k * block_size + i];
    			}

    			stripe = (k - rs_disk1 + p) % p;
    			for( i = 0; i < block_size; i++)
    			{
    				tmp_for_s[i] = check_data[rs_disk1][k * block_size + i] ^ tmp[stripe * block_size + i];
    			}
    		}		
    	}
    	free(tmp);
    	free(tmp_for_s);
    }

    /**
	 *  restore data like evenodd
	 */
    void Evenodd_decoding( int *restarts)
    {
    	int i,j,stripe,k;
    	int rs_disk1 = -1;
    	int rs_disk2 = -1;
    	int rs_nbr = 0;

    	for(i = 0; i < data_disk_nbr + 2; i++)
    	{
    		if( restarts[i] == 1)
    		{
    			rs_disk1 = i;
    			rs_nbr ++;
    			break;
    		}
    	}
    	if(rs_disk1 != -1)
    	{
    		for( i = rs_disk1 + 1; i < data_disk_nbr + 2; i++)
    		{
    			if(restarts[i] == 1)
    			{
    				rs_disk2 = i;
    				rs_nbr ++;
    				break;
    			}
    		}
    	}
    	if(rs_disk1 != -1)memset(check_data[rs_disk1], 0 ,check_data_size * sizeof(int));
    	if(rs_disk2 != -1)memset(check_data[rs_disk2], 0 ,check_data_size * sizeof(int));

    	if(rs_disk1 >= data_disk_nbr)
    	{
    		if(restarts[data_disk_nbr] == 1)/*row checksum*/
    		{
    			STAR_encoding_row();
    		}
    		if(restarts[data_disk_nbr + 1] == 1)/*slope 1*/
    		{
    			STAR_encoding_diag1();
    		}
    	}

    	if(rs_disk1 < data_disk_nbr && rs_nbr == 1)
    	{
    		for( i = 0; i < check_data_size; i++) 
    		{
    			for( j = 0; j <= data_disk_nbr; j++) 
    			{
    				if( j !=  rs_disk1)
    				{
    					check_data[rs_disk1][i] ^= check_data[j][i];
    				}
    			}
    		}
    	}
        
        //*one data error + one checksum error*/
        if(rs_nbr == 2 && rs_disk1 < data_disk_nbr && rs_disk2 >= data_disk_nbr)
    	{
    		if( rs_disk2 == data_disk_nbr + 1)/*slope -1*/
    		{
    			for(i = 0; i < check_data_size; i++)
    				for(j = 0; j <= data_disk_nbr; j++)
    					if( j != rs_disk1)
    						check_data[rs_disk1][i] ^= check_data[j][i];
    			STAR_encoding_diag1();
    		}

    		if( rs_disk2 == data_disk_nbr)
    		{
    			/*int i,j,stripe,k;*/
    			int *tmp;
    			tmp = (int*)malloc(sizeof(int) * (p * block_size));
    			memset(tmp, 0, p * block_size * sizeof(int));

    		    int *tmp_for_s;
    			tmp_for_s = (int *)malloc(sizeof(int) * block_size);
    			memset(tmp_for_s, 0, block_size *sizeof(int));
    	
    			for(stripe = 0; stripe < block_nbr+1; stripe++)
    			{
    				for(i = 0; i < data_disk_nbr ; i++)
    				{
    					for(j = 0; j<block_size; j++)
    					{
    						k = (stripe - i + p) % p;
    						if( k < block_nbr)
    							tmp[stripe * block_size + j] ^= check_data[i][ k * block_size + j];
    					}
    				}
    			}

    			stripe = (rs_disk1 + p - 1) % p;
    			for( i = 0; i < block_size; i++)
    			{
    				if( stripe == p - 1)
    					tmp_for_s[i] = tmp[stripe * block_size + i];
    				else
    					tmp_for_s[i] = tmp[stripe * block_size + i] ^ check_data[data_disk_nbr + 1][stripe * block_size + i];
    			}
    			
    			for( i = 0; i < block_nbr; i++)
    			{
    				for( j = 0; j < block_size; j++)
    				{
    					tmp[i * block_size + j] ^= (tmp_for_s[j] ^ check_data[data_disk_nbr + 1][ i * block_size + j]);
    				}
    			}
    			for( j = 0; j < block_size; j++)
    				tmp[ block_nbr * block_size + j] ^= tmp_for_s[j];
    				
    			for( i = 0; i < p; i++)
    			{
    				j = (i + p - rs_disk1) % p;
    				if( j < p - 1)
    				{
    					memmove( check_data[rs_disk1] + j * block_size, tmp + i * block_size, block_size * sizeof(int));
    																													 
    				}
    			}

    			free(tmp);
    			free(tmp_for_s);
    			STAR_encoding_row();
    		}
    	}

    	if(rs_nbr == 2 && rs_disk1 < data_disk_nbr && rs_disk2 < data_disk_nbr)/*2 data error */
    	{
    		
    		int *tmp;
    		tmp = (int*)malloc(sizeof(int) * (p * block_size));
    		memset(tmp, 0, p*block_size*sizeof(int));

    		/*computing s firstly*/
    		int *tmp_for_s;
    		tmp_for_s = (int *)malloc(sizeof(int) * block_size);
    		memset(tmp_for_s, 0, block_size *sizeof(int));
    		for( i = 0; i < block_nbr; i++)
    		{	
    			for( j = 0; j < block_size; j++)
    			{
    				tmp_for_s[j] ^= check_data[data_disk_nbr][i * block_size + j];
    				tmp_for_s[j] ^= check_data[data_disk_nbr + 1][i * block_size + j];
    			}
    		}


    		for(stripe = 0; stripe < block_nbr+1; stripe++)
    		{
    			for(i = 0; i < data_disk_nbr; i++)
    			{
    				k = (stripe - i + p) % p;
    				if( k < block_nbr)
    					for(j = 0; j<block_size; j++)
    					{				
    						tmp[stripe * block_size + j] ^= check_data[i][ k * block_size + j];
    					}
    			}
    		}
    		for( i = 0; i< block_nbr ; i++)
    		{
    			for( j = 0; j< block_size; j++)
    			{
    				tmp[ i * block_size + j] ^= (check_data[data_disk_nbr + 1][i * block_size + j] ^ tmp_for_s[j]);
    			}
    		}
    		for( j = 0; j < block_size; j++)
    			tmp[ block_nbr * block_size + j] ^= tmp_for_s[j];
            
    		/*store s~ in tmp*/	
    		stripe = (rs_disk1 + p - 1) % p;
    		memmove( tmp_for_s, tmp + stripe * block_size, block_size * sizeof(int));

    		while(1)
    		{
    			k = (stripe - rs_disk2 + p) % p;
    			if( k == block_nbr)
    				break;
    			memmove( check_data[rs_disk2] + k * block_size, tmp_for_s, block_size * sizeof(int));

    			for( j = 0; j < block_size; j++)
    			{
    				for( i = 0; i <= data_disk_nbr; i++)
    					if( i != rs_disk1)
    						check_data[rs_disk1][k * block_size + j] ^= check_data[i][k * block_size + j] ;
    			}
                
    			stripe = (k + rs_disk1 + p) % p;

    			for( j = 0; j < block_size; j++)
    			{
    				tmp_for_s[j] = check_data[rs_disk1][k * block_size + j] ^ tmp[stripe * block_size + j];
    			}

    		}

    		free(tmp);
    		free(tmp_for_s);
    	}

    }
};
