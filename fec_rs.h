#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Erasure code Reed-Solomon. C++ version
 * @author Roger Song
 *
 */
using namespace std;

#define prim_poly_32 020000007
#define prim_poly_16 0210013
#define prim_poly_8 0435
#define prim_poly_4 023
#define prim_poly_2 07

#ifdef W_8
  static int Modar_w = 8;
  static int Modar_nw = 256;
  static int Modar_nwm1 = 255;
  static int Modar_poly = prim_poly_8;
 #endif

#ifdef W_16
  static int Modar_w = 16;
  static int Modar_nw = 65536;
  static int Modar_nwm1 = 65535;
  static int Modar_poly = prim_poly_16;
#endif

static int *B_TO_J;
static int *J_TO_B;
static int Modar_M;
static int Modar_N;
static int Modar_Iam;

typedef struct {
  int *condensed_matrix;   /* The n*n dispersal matrix with rows deleted */
  int *row_identities;     /* A n x 1 vector of the original row identities of the cond_matrix */
} Condensed_Matrix;


class rscode {
    
private:
    unsigned int num; // original data cols num
    char **rs; // original data
    static const int FT_NUM = 2; //default checksum num
    int gf_already_setup;
    unsigned int allNum;
    static const int DATA_LENGTH = 1024; // default stripe size
    int stripe_unit_size;  // stripe size
    int rsNum;
    int *inthis;
public:
    rscode()
    {
        allNum = 6;
        rsNum = FT_NUM;
        num = allNum - rsNum;
        stripe_unit_size = DATA_LENGTH;
        gf_already_setup = 0;

        // use allnum here, rs includes original data and redudant data
        rs = (char **) malloc(sizeof(char *)*allNum);  
        for(int i=0;i<allNum;i++) {
	        rs[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(rs[i],0,stripe_unit_size);
	    }  

        inthis = new int[allNum];
        for(int i = 0; i < allNum; i++)
        {
            inthis[i] = 1;
        }
    }

    rscode(int allnum, int rsnum, int dataLength)
    {
        allNum = allnum;
        rsNum = rsnum;
        num = allNum - rsNum;
        stripe_unit_size = dataLength;
        gf_already_setup = 0;
        
        // use allnum here, rs includes original data and redudant data
        rs = (char **) malloc(sizeof(char *)*allNum);  
        for(int i=0;i<allNum;i++) {
	        rs[i]=(char *) malloc(sizeof(char)*stripe_unit_size);
	        memset(rs[i],0,stripe_unit_size);
	    }  
        inthis = new int[allNum];
        for(int i = 0; i < allNum; i++)
        {
            inthis[i] = 1;
        }
    }

    ~rscode()
    {
        for(int i=0;i<num;i++) {
	        free(rs[i]);
	    }  
        free(rs);

        delete[] inthis;
    }

    void setErrData(int *err)
    {
        for(int i = 0; i < allNum; i++ )
        {
            inthis[i] = err[i];
        }
    }

    
    /**
	*  an easy test case
	*/
	void setData()
	{
		for(int i = 0; i < num; i++){
			for(int j = 0; j < stripe_unit_size; j++){
				rs[i][j]=(char) ('a' + i);
			}
		}
	}

    /**
	 * for testing and debug.
	 */
	void outputOrigin(){
		
		printf("After decoding:");
		for(int i=0; i < num; i++)
		{
			printf("data:%d:  ",i);
			printf(rs[i]);
            printf("\n");
		}
	}

    /**
	 * for testing and debug.
	 */
	void outputOdata()
	{
		
		printf("The res:");
        for(int i = num; i < allNum; i++)
        {
            printf(rs[i]);
            printf("\n");
        }
	}

    /**
	 * rs encoding main function. 
	 * there is a simple testcase in setData func 
	 */
	 void encoding_rs()
     {
        int cols,rows;
        int factor=0,*factors;
        int *vdm,tmp;
        int z=rsNum,n=0;
        unsigned int fl=(num-1);
        int i=0,j=0;
        char *block;
	    char **buffer;
        int l=0;
        int numx=num;

        n=num;
	    cols=n;
	    rows=z+n;
        factors = (int *) malloc(sizeof(int)*n);
        block=(char *) malloc(sizeof(char)*stripe_unit_size);
        buffer = (char **) malloc(sizeof(char *)*n);
        for (int i = 0; i < n; i++) {
            buffer[i] = (char *) malloc(stripe_unit_size);
	        memset(buffer[i], 0, stripe_unit_size);
        }

        for(int i=0;i<n;i++)
        {
     	   for(int l=0;l<DATA_LENGTH;l++)
     	   {
    		   buffer[i][l]=rs[i][l];
     	   }
        }

        for (int i = 0; i < n; i++){ 
            factors[i] = 1;
        }

        vdm = gf_make_dispersal_matrix(rows, cols);

        for (int i = cols ;i< rows; i++) {         
  	        memset(block, 0, stripe_unit_size); 
	        for (int j = 0; j < cols; j++) {
                tmp = vdm[i*cols+j]; 
	            if (0 != tmp) {
                    factor = gf_single_divide(tmp, factors[j]);
            //		printk("factor: %d\n",factor);
                    factors[j] = tmp;
                    gf_mult_region(buffer[j], stripe_unit_size, factor);
            //		printk("mult hou the rs is %s\n",buffer[j]);
                    gf_add_parity(buffer[j], block, stripe_unit_size);// do parity check
            //      printk("the block is %s",block);
	        	}
	    	}
		
            memcpy(rs[numx],block,stripe_unit_size);
            numx++;
  	    }

        for(int i=0;i<num;i++)
        {
            if(buffer[i]==NULL) {
	            printf(" already free ** \n ");
	            continue;
            }
            free(buffer[i]);
        }
        if(buffer!=NULL){ 
            free(buffer);
        }
     }

    void decoding_rs()
    {
        int *vdm;
        Condensed_Matrix *cm;
        int rows,cols;
        int m,n,tmp,factor;
        int *exists,*factors,*map;
        char **buffer,*block;
        int *id,*mat,*inv,k;
        char **buff;
        int index,i,j,ret;
        int xx=0;
        int js=0;
        int l=0;

        buff=(char **) malloc(sizeof(char *)*allNum); 
        for (int i = 0; i < (allNum); i++) {
            buff[i] = (char *) malloc(stripe_unit_size);
            memset(buff[i],0,stripe_unit_size);
        }

        m=rsNum;
        n=num;
        cols=n;
        rows=m+n;
        vdm = gf_make_dispersal_matrix(rows, cols);
        exists = (int *) malloc(sizeof(int) * rows);

        factors = (int *) malloc(sizeof(int) * rows);

        map = (int *) malloc(sizeof(int) * rows);
        buffer = (char **) malloc(sizeof(char *)*(n+m));
        for (int i = 0; i < (n+rsNum); i++) {
            buffer[i] = (char *) malloc(stripe_unit_size);
            memset(buffer[i],0,stripe_unit_size);
        }
        
        for(j=0;j<(num+rsNum);j++){
        	for(i=0;i<stripe_unit_size;i++)
        	{
        	    buff[j][i] = rs[j][i];
        	} 
        }

        j=0;
        for (int i = 0; i <rows && j <cols; i++) 
  	    {
  	        if (inthis[i]==0){
               map[i]=-1;
  	        }
	        else{
               map[i] = j++;
               for(int l=0;l<stripe_unit_size;l++){ 
                   buffer[map[i]][l]=buff[i][l]; 
               }
            }
  	    }
        
        j = 0;
        for (int i = 0; i < cols; i++){
            if (map[i] == -1){ 
                j++;  //map == -1 means data loss or data corruption
            }
        }

        printf("Blocks to decode: %d\n", j);
        block = (char *) malloc(sizeof(char)*stripe_unit_size);
        for (int i = 0; i < rows; i++){
            exists[i] = (map[i] != -1);
        }
        cm = gf_condense_dispersal_matrix(vdm, exists, rows, cols);
        mat = cm->condensed_matrix;
        id = cm->row_identities;
        for (int i = 0; i < cols; i++) {
            if (map[i] == -1){ 
                map[i] = map[id[i]];
            }
        }

        inv = gf_invert_matrix(mat, cols);
        int hh=0;
        for(int i = 0; i < rows; i++){
            factors[i] = 1;
        }

        for (int i = 0; i < cols ; i++) 
        {
            if (id[i] >= cols) {
                memset(block, 0, stripe_unit_size);
                for (int j = 0; j < cols; j++) {
                    tmp = inv[i*cols+j];
                    factor = gf_single_divide(tmp, factors[j]);

                    factors[j] = tmp;
                    gf_mult_region(buffer[map[j]], stripe_unit_size, factor);

                    gf_add_parity(buffer[map[j]], block, stripe_unit_size);
      	        }

                memcpy(rs[i],block,stripe_unit_size);
            }

        }

        for(int i=0;i<(num+rsNum);i++)
        {
            if(buff[i]==NULL) {
	            continue;
	        }
            free(buff[i]);
        }
        if(buff!=NULL) free(buff); 	

        for(int i=0;i<(num+rsNum);i++)
        {
            if(buffer[i]==NULL) {
	        continue;
	        }
            free(buffer[i]);
        }
        if(buffer!=NULL){ 
            free(buffer);
        } 	

    }

    string code_show(){

        return "rs";
    }

    void gf_modar_setup()
    {
        int j, b, t;
        if (gf_already_setup) return;
        B_TO_J = (int *) malloc(sizeof(int)*256);

        if (B_TO_J == NULL) {
            printf("gf_modar_setup, malloc B_TO_J");
        }
        /* When the word size is 8 bits, make three copies of the table so that
          you don't have to do the extra addition or subtraction in the
          multiplication/division routines */

        J_TO_B = (int *) malloc(sizeof(int)*256*3);
        if (J_TO_B == NULL) {
            printf("gf_modar_setup, malloc J_TO_B");
        }
        for (j = 0; j < 256; j++) {
            B_TO_J[j] = 255;
            J_TO_B[j] = 0;
        } 

        b = 1;
        for (j = 0; j < 255; j++) {
            if (B_TO_J[b] != 255) {
                printf( "Error: j=%d, b=%d, B->J[b]=%d, J->B[j]=%d (0%o)\n");
            }
            B_TO_J[b] = j;
            J_TO_B[j] = b;
            b = b << 1;
            if (b & 256){
                b = (b ^ prim_poly_8) & 255;
            }
        }

        for (j = 0; j < 255; j++) {
            J_TO_B[j+255] = J_TO_B[j];
            J_TO_B[j+2*255] = J_TO_B[j];
        }
        J_TO_B += 255;


        gf_already_setup = 1;

    }


    int gf_single_multiply(int xxx, int yyy)
    {
        unsigned int sum_j;
        unsigned char zzz;

        gf_modar_setup();
        if (xxx == 0 || yyy == 0) {
            zzz = 0;
        } else {
            sum_j = (int) (B_TO_J[xxx] + (int) B_TO_J[yyy]);
            zzz = J_TO_B[sum_j];
        }
        return zzz;
    }
    int gf_single_divide(int a, int b)
    {
        int sum_j;
        gf_modar_setup();
        if (b == 0) 
            return -1;
        if (a == 0) 
            return 0;
        sum_j = B_TO_J[a] - B_TO_J[b];

        return (int) J_TO_B[sum_j];
    }



    void gf_mult_region(void *region, int size, int factor)
    {
        int sum_j;
        int flog;
        unsigned char *r;
        int sz;
        int r_cache;

        gf_modar_setup();

        if (factor == 1)
            return;
        if (factor == 0) {
            (void) memset(region, 0, size);
            return;
        }

        flog = B_TO_J[factor];
        sz = size / sizeof(unsigned char);
        r = ((unsigned char *) region) + sz;

        while (r != region) {
            r--;
            r_cache = *r;
            if (r_cache != 0) {
              sum_j = (int) (B_TO_J[r_cache] + flog);

              *r = J_TO_B[sum_j];
            }
        }
    }

    void gf_fast_add_parity(void *to_add, void *to_modify, int size)
    {
        unsigned long *p, *t;
        int j;
        j = size/sizeof(unsigned long);
        t = (unsigned long *) to_add;
        p = (unsigned long *) to_modify;
        p += j;
        t += j;
        while(t != (unsigned long *) to_add) {
            p--;
            t--;
            *p = *p ^ *t;
        }
    }

    void gf_add_parity(void *to_add, void *to_modify, int size)
    {
        unsigned long ta, tm;
        unsigned char *cta, *ctm;
        int sml;

        if (size <= 0) 
            return;

        ta = (unsigned long) to_add % sizeof(unsigned long);
        tm = (unsigned long) to_modify% sizeof(unsigned long);
        if (ta != tm) {
            printf("Error: gf_add_parity: to_add and to_modify are not aligned\n");
        }

        cta = (unsigned char *) to_add;
        ctm = (unsigned char *) to_modify;

     //   printf("in add_parity 1\n");

      /* Align to long boundary */
        if (ta != 0) {
            while (ta != sizeof(unsigned long) && size > 0) {
                *ctm = *ctm ^ *cta;
                ctm++;
                cta++;
                size--;
                ta++;
            }
        }
      
        if (size == 0)
            return;

        /* Call gf_fast_add_parity to do it fast */

        sml = size / sizeof(unsigned long);     
    	  
        if (sml > 0){
            gf_fast_add_parity(cta, ctm, size);
        }
        size -= sml * sizeof(unsigned long);

        if (size == 0){
            return;
        }

        /* Do the last few bytes if they are unalighed */
        cta += sml * sizeof(unsigned long);
        ctm += sml * sizeof(unsigned long);

        while (size > 0) {
            *ctm = *ctm ^ *cta;
            ctm++;
            cta++;
            size--;
        }
    }

    int gf_log(int value)
    {
       return B_TO_J[value];
    }

    /* This returns the rows*cols vandermonde matrix.  N+M must be
       < 2^w -1.  Row 0 is in elements 0 to cols-1.  Row one is 
       in elements cols to 2cols-1.  Etc.*/

    int *gf_make_vandermonde(int rows, int cols)
    {
        int *vdm, i, j, k;
        void gf_modar_setup();

        if (rows >= 255 || cols >= 255) {
            printf("Error: gf_make_vandermonde: %d + %d >= %d\n");
        }
     
        vdm = (int *) malloc(sizeof(int) * rows * cols);
        if (vdm == NULL) {
            printf("Malloc: Vandermonde matrix");
        }
        for (i = 0; i < rows; i++) {
            k = 1;
            for (j = 0; j < cols; j++) {
                vdm[i*cols+j] = k;
                k = gf_single_multiply(k, i);
            }
        }
        return vdm;
    }

    static int find_swap_row(int *matrix, int rows, int cols, int row_num)
    {
        int j;

        for (j = row_num; j < rows; j++) {
            if (matrix[j*cols+row_num] != 0) return j;
        }
        return -1;
    }


    int *gf_make_dispersal_matrix(int rows, int cols)
    {
        int *vdm, i, j, k, l, inv, tmp, colindex;

        vdm = gf_make_vandermonde(rows, cols);


        for (i = 0; i < cols && i < rows; i++) {
            j = find_swap_row(vdm, rows, cols, i);
            if (-1 == j) {
                printf("Error: make_dispersal_matrix.  Can't find swap row %d\n");
            }

            if (j != i) {
                for (k = 0; k < cols; k++) {  
                    tmp = vdm[j*cols+k];
                    vdm[j*cols+k] = vdm[i*cols+k];
                    vdm[i*cols+k] = tmp;
                }
            }
            if (vdm[i*cols+i] == 0) {
                printf("Internal error -- this shouldn't happen\n");
            }

            if (vdm[i*cols+i] != 1) {
                inv = gf_single_divide(1, vdm[i*cols+i]);
                k = i;
                for (j = 0; j < rows; j++) {
                    vdm[k] = gf_single_multiply(inv, vdm[k]);
                    k += cols;
                }

            }
            if (vdm[i*cols+i] != 1) {
                printf("Internal error -- this shouldn't happen #2)\n");
            }

            for (j = 0; j < cols; j++) {
                colindex = vdm[i*cols+j];
                if (j != i && colindex != 0) {
                    k = j;
                    for (l = 0; l < rows; l++) {
                        vdm[k] = vdm[k] ^ gf_single_multiply(colindex, vdm[l*cols+i]);
                        k += cols;
                    }
                }
            }
        }

        return vdm;
    }



    Condensed_Matrix *gf_condense_dispersal_matrix(
                        int *disp, int *existing_rows, int rows,    int  cols)
    {
        Condensed_Matrix *cm;
        int *m;
        int *id;
        int i, j, k, tmp;

        /* Allocate cm and initialize */
        cm = (Condensed_Matrix *) malloc(sizeof(Condensed_Matrix)); 
        if (NULL == cm)
        {
            printf("gf_condense_dispersal_matrix - Condensed_Matrix"); 
        }
        cm->condensed_matrix = (int *) malloc(sizeof(int)*cols*cols);
        if (NULL == cm->condensed_matrix) { 
            printf("gf_condense_dispersal_matrix - cm->condensed_matrix"); 
       
        }
        cm->row_identities = (int *) malloc(sizeof(int)*cols);
        if (NULL == cm->row_identities) 
        { 
            printf("gf_condense_dispersal_matrix - cm->row_identities"); 
        }
        m = cm->condensed_matrix;
        id = cm->row_identities;
        for (i = 0; i < cols; i++){
            id[i] = -1;
        }

        /* First put identity rows in their proper places */

        for (i = 0; i < cols; i++) {
            if (existing_rows[i] != 0) {
                id[i] = i;
                tmp = cols*i;
                for (j = 0; j < cols; j++){
                    m[tmp+j] = disp[tmp+j];
                }
            }
        }

      /* Next, put coding rows in */
      k = 0;
      for (i = cols; i < rows; i++) {
        if (existing_rows[i] != 0) {
            while(k < cols && id[k] != -1){ 
                k++;
            }
            if (k == cols){ 
                return cm;
            }
            id[k] = i;
            for (j = 0; j < cols; j++)
            {
                m[cols*k+j] = disp[cols*i+j];
            }
        }
      }

      /* If we're here, there are no more coding rows -- check to see that the
         condensed dispersal matrix is full -- otherwise, it's not -- return an
         error */

      while(k < cols && id[k] != -1){
          k++;
      }
      if (k == cols){ 
          return cm;
      }

      free(id);
      free(m);
      free(cm);
      return NULL;
    }


    int *gf_invert_matrix(int *mat, int rows)
    {
        int *inv;
        int *copy;
        int cols, i, j, k, x, rs2;
        int row_start, tmp, inverse;
     
        cols = rows;

        inv = (int *) malloc(sizeof(int)*rows*cols);
        if (inv == NULL)
        { 
            printf("gf_invert_matrix - inv");  
        }
        copy = (int *) malloc(sizeof(int)*rows*cols);
        if (copy == NULL) 
        {
            printf("gf_invert_matrix - copy"); 
        }

        k = 0;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                inv[k] = (i == j) ? 1 : 0;
                copy[k] = mat[k];
                k++;
            }
        }

        /* pic(inv, copy, rows, "Start"); */

        /* First -- convert into upper triangular */
        for (i = 0; i < cols; i++) {
            row_start = cols*i;

            /* Swap rows if we ave a zero i,i element.  If we can't swap, then the 
            matrix was not invertible */

            if (copy[row_start+i] == 0) { 
                for (j = i+1; j < rows && copy[cols*j+i] == 0; j++) ;
                if (j == rows) {
                    printf( "gf_invert_matrix: Matrix not invertible!!\n");
                    exit(1);
                }
                rs2 = j*cols;
                for (k = 0; k < cols; k++) {
                    tmp = copy[row_start+k];
                    copy[row_start+k] = copy[rs2+k];
                    copy[rs2+k] = tmp;
                    tmp = inv[row_start+k];
                    inv[row_start+k] = inv[rs2+k];
                    inv[rs2+k] = tmp;
                }
            }
     
            /* Multiply the row by 1/element i,i */
            tmp = copy[row_start+i];
            if (tmp != 1) {
                inverse = gf_single_divide(1, tmp);
                for (j = 0; j < cols; j++) { 
                    copy[row_start+j] = gf_single_multiply(copy[row_start+j], inverse);
                    inv[row_start+j] = gf_single_multiply(inv[row_start+j], inverse);
                }
                /* pic(inv, copy, rows, "Divided through"); */
            }

            /* Now for each j>i, add A_ji*Ai to Aj */
            k = row_start+i;
            for (j = i+1; j != cols; j++) {
                k += cols;
                if (copy[k] != 0) {
                    if (copy[k] == 1) {
                        rs2 = cols*j;
                        for (x = 0; x < cols; x++) {
                            copy[rs2+x] ^= copy[row_start+x];
                            inv[rs2+x] ^= inv[row_start+x];
                        }
                    } 
                    else {
                        tmp = copy[k];
                        rs2 = cols*j;
                        for (x = 0; x < cols; x++) {
                            copy[rs2+x] ^= gf_single_multiply(tmp, copy[row_start+x]);
                            inv[rs2+x] ^= gf_single_multiply(tmp, inv[row_start+x]);
                        }
                    }   
                }
            }
            /* pic(inv, copy, rows, "Eliminated rows"); */
        }

        /* Now the matrix is upper triangular.  Start at the top and multiply down */

        for (i = rows-1; i >= 0; i--) {
            row_start = i*cols;
            for (j = 0; j < i; j++) {
                rs2 = j*cols;
                if (0 != copy[rs2+i]) {
                    tmp = copy[rs2+i];
                    copy[rs2+i] = 0; 
                for (k = 0; k < cols; k++) {
                    inv[rs2+k] ^= gf_single_multiply(tmp, inv[row_start+k]);
                }
            }
        }
        /* pic(inv, copy, rows, "One Column"); */
      }
      free(copy);
      return inv;
    }

    int *gf_matrix_multiply(int *a, int *b, int cols)
    {
        int *prod, i, j, k;

        prod = (int *) malloc(sizeof(int)*cols*cols);
        if (prod == NULL) 
        { 
            printf("gf_matrix_multiply - prod"); 
        }

        for (i = 0; i < cols*cols; i++){
            prod[i] = 0;
        }

        for (i = 0; i < cols; i++) {
            for (j = 0; j < cols; j++) {
                for (k = 0; k < cols; k++) {
                    prod[i*cols+j] ^= gf_single_multiply(a[i*cols+k], b[k*cols+j]);
                }
            }
        }
        return prod;
    }
};
