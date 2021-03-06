/*$Id: makemould.cxx 1086 2007-10-10 10:55:39Z  $*/
/************************************************************/
/*    makemould.C                                           */
/*Program to output a binary file of a mould/casting domain */ 
/* for very simple geometries                               */
/************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <gd.h>

/* definition of a mould domain description object */
class Size {
public:
  int xsize;
  int ysize;
  int ncells;
  int mouldx;
  int mouldy;
  int mouldstartx;
  int mouldstarty;
  int initSize();
  void createMould(unsigned char * mould);
  Size();
  virtual ~Size();
}; /* end of class Size */

/* default constructor for mould domain description */
Size::Size(){
     xsize=0;
     ysize=0;
     ncells=0;
     mouldx=0;
     mouldy=0;
     mouldstartx=0;
     mouldstarty=0;
}/* end of Size constructure */

/* destructor for mould domain description (Size object) */
Size:: ~Size(){
}; /* end of Size destructor */

/***************************************/
/* initialize a mould domain structure */
/***************************************/
int Size::initSize(){
   /* this is hard coded for now */
   /* TODO: input the relevant options  from commandline */
   /* or a ctrl file?? */
   xsize = 250;
   ysize = 250;
   mouldx = 250;
   mouldy = 1;
   ncells = xsize*ysize;
   return ncells;
}/* end of initSize */

/*************************************/
/* set the values in the mould array */
/*************************************/
void Size::createMould(unsigned char * mould){
   /* this only creates a rectangle starting at the origin */
   /* TODO: program in more complicated shapes as needed   */
   int i,j;
   /*test the values*/
   /* should be tested on input however */
   /* TODO: test values on input?       */
   /* or a seperate value-test method?  */

   if ((mouldstartx+mouldx > xsize) || (mouldstarty > ysize )){
      fprintf(stderr,"ERROR:createMould: mould exceeds model domain.\n");
      exit (100);
   }

   for (i=0;i<xsize;i++){
      for(j=0;j<ysize;j++){
         if (i< mouldx && j < mouldy){
           *(mould + xsize*j +i) = 255;
          }
       }
    }
}/* end of createMould */

/*********************************/
/*  write the mould into a png   */
/*  image file using the gd lib  */
/*********************************/

/* this is deliberately coded to be compatible with plain c     */
/* so that the routine could be used without rewriting in both  */
/* c and c++ programs (Size could be a structure containing the */
/* suitable numbers xsize and ysize rather than a c++ class     */

   /* Assumes mould array is the right size!*/
void write_image(Size domain,unsigned char * mould){
   gdImagePtr im_out;
   char  out_fname[255];
   FILE * outfile;
   int pngcolours[32];
   int index;
   int i,j;

      /* initialize the image structure */
      im_out=gdImageCreate(domain.xsize,domain.ysize);

      /* set up the colour table                 */
      /* just 32 colours for simplicity          */
      pngcolours[0]=gdImageColorAllocate(im_out,255,255,255);

      /* mould (colour index 31) is an orangey colour in this setup */
      pngcolours[31]=gdImageColorAllocate(im_out,255,85,85);
      for(i=1;i<31;i++){
         pngcolours[i]=gdImageColorAllocate(im_out,255 - 8*i,255-8*i,255-8*i);
      }

   /* transfer the pixels */
   for (i=0;i<domain.xsize;i++){
      for(j=0;j<domain.ysize;j++){
          /* create an index in the range 0->31 */
          index = (*(mould+domain.xsize * j + i) >> 3) ;
          gdImageSetPixel(im_out,i,j,pngcolours[index]);
       }
    }/* finished transferring the pixels */

    /* write out the gd image in png format */
    gdImageInterlace(im_out,1);
    snprintf(out_fname,254,"%s.png","mould");
    outfile=fopen(out_fname,"w");
    gdImagePng(im_out,outfile);
    fclose(outfile);
}/* end of write_image */



/*************************************/
/*                                   */
/* main procedure - get options and  */
/* output the final file             */
/*                                   */
/*************************************/
int main(){
   unsigned char * mould;
   int i,j;
   int ncells;

   Size domain;

   FILE * fp;

   /* set up the size amounts */ 
   ncells=domain.initSize();

   /* create the storage array */
   /* just easier if mould is a top-level variable */
   mould = (unsigned char *) calloc (domain.ncells+1,sizeof(unsigned char));

   domain.createMould(mould);
   /* create the binary file */
   fp = fopen("mould.inp","w");
   fwrite (mould,sizeof(unsigned char),domain.ncells,fp);
   fclose (fp);

   /* also create an easily viewable image format */
   write_image(domain,mould);

   free(mould);

}/* end of main procedure for makemould */


