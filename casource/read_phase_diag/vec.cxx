// vec.cpp: implementation of the vec class.
//
//////////////////////////////////////////////////////////////////////

/*$Id: vec.cxx 1086 2007-10-10 10:55:39Z  $*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec.h"
int countmat=1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vec::vec()
{
 x=0;
 y=0;
 z=0;

}

vec::vec(FLOAT incol,FLOAT inrow,FLOAT inz)
{
	x = incol;
	y = inrow;
	z = inz;
}


vec::~vec()
{

}

void vec::print(FILE * fp)
{
	fprintf(fp,"%g,%g,%g",x,y,z);
}

vec vec::operator *(FLOAT mul)
{
	vec res(x,y,z);
	res.x *= mul;
	res.y *= mul;
	res.z *= mul;
	return(res);
}



FLOAT vec::operator !()//returns the norm
{
	return (FLOAT)sqrt(x*x + y*y + z*z);
}

void vec::zerovec(){
   x=0;
   y=0;
   z=0;
}



//basic vector arithmetic
vec vec::operator +(vec av)  //add
{
   vec res(x+av.x,y+av.y,z+av.z);
   return (res);
}
vec vec::operator -(vec av)  //subtract
{
   vec res(x-av.x,y-av.y,z-av.z);
   return (res);
}
FLOAT vec::operator *(vec dp) //dot product
{
	return (FLOAT)(x*dp.x+y*dp.y+z*dp.z);
}
vec vec::operator ^(vec xp) //cross product
{
   vec res((y*xp.z) - (z*xp.y), (z*xp.x)-(x*xp.z),(x*xp.y)-(y*xp.x));
	return (res);
}

void vec::list(FILE *fp)
{
	//fprintf(fp,"%.5g,%.5g",x,y);
	char * ls;
	ls = strlist();
	fprintf(fp,"%s",ls);
	free(ls);

}

char * vec::strlist()
{
	char * res;
	res = (char *) calloc(MAXLEN,sizeof(char));
	snprintf(res,MAXLEN,"%.5g,%.5g,%.5g,%.5g",x,y,z,!(*this));
	return (res);
}
Mat::Mat(){
   init_Mat(2,2);
}
Mat::Mat(int nc){
   init_Mat(nc,nc);
}
Mat::Mat(int nr,int nc){
   init_Mat(nr,nc);
}
Mat::Mat(const Mat & am){
   init_Mat(am.nrow,am.ncol);
   memcpy(data,am.data,ndata*sizeof(FLOAT));
}

void Mat::init_Mat( int rows, int cols){
   ncol = cols;
   nrow = rows;
   my_num=countmat++;
   printf("Creating matrix id:%i\n",my_num);
   if (ncol == nrow) isSquare=1;
   else isSquare=0;
   ndata = ncol * nrow;
   data =  (FLOAT *) calloc(ndata, sizeof(FLOAT));
}
Mat::~Mat(){
   ncol = 0;
   nrow = 0;
   ndata = 0;
   printf("Destroying matrix id:%i\n",my_num);
   free(data);
}

/* Replace old data with given data */
void Mat::set_Mat(FLOAT * indata){
   memcpy(data,indata,ndata*sizeof(FLOAT));
}
const Mat &  Mat::operator =(const Mat &  am){
   if (ncol != am.ncol || nrow != am.nrow){
      fprintf(stderr,"Matrix equal: bad sizes\n");
   }else{
       memcpy(data,am.data,ndata*sizeof(FLOAT));
   }
return(*this);
}


Mat  Mat::operator +(Mat  am){
   Mat  res (ncol,nrow);
   int i;
   if (ncol != am.ncol || nrow != am.nrow){
      fprintf(stderr,"Matrix add: bad sizes\n");
   }else{
      for(i=0;i<ndata;i++){
         res.data[i] = data[i]+am.data[i];
      }
   }
   return(res);
 }

Mat  Mat::operator *(Mat  am){
   Mat res (nrow,am.ncol);
   int i,j,k;
   int res_idx;
   res.printMat(stdout);
   if (ncol != am.nrow){
      fprintf(stderr,"Matrix mult: bad sizes\n");
   }else{
      for(j=0;j<nrow;j++){
         for(k=0;k<am.ncol;k++){
            res_idx = j* res.nrow + k;
            printf("j %i k %i res_idx:%i\t",j,k,res_idx);
            for(i=0;i<ncol;i++){
               printf("[i:%i\td:%i\ta:%i]\t",i,i+j*ncol,k+i*am.ncol);

               res.data[res_idx] += data[i+j*ncol] * am.data[k+i*am.ncol];
            }
            printf("\n");
         }
      }
   }
   res.printMat(stdout);
   return(res);
 }

Mat  Mat::operator *(FLOAT sc){
   Mat res(ncol,nrow);
   int i;
   for(i=0;i<ndata;i++){
      res.data[i] = data[i]* sc;
   }
   return(res);
 }

void Mat::printMat(FILE * fp){
   int i,j;
   FLOAT *datap=data;
   fprintf(fp,"%i\n",my_num);
   for (j=0; j<nrow;j++){
      for(i=0;i<ncol;i++){
         fprintf(fp,"%g\t",*datap++);
      }
      fprintf(fp,"\n");
   }
}


   








