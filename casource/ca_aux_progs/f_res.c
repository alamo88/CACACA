/* make a tecplot data file from binary output file */
/* needs debin.c and machine.h */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <gd.h>
#include "../colour.h"
#include "../machine.h"
#include "debin.h"

#ifndef TRUE
#define TRUE 1 
#endif
#ifndef MAX_WORD_LENGTH
#define MAX_WORD_LENGTH 255
#endif
typedef enum colour_num{
COLOUR_REDBLUE_POS,COLOUR_BW_POS} Colours;
/************************/
/* scale the image data */
/* according to the     */
/* selected options     */
/************************/
int scale_png(float pngmin,float pngmax,CA_FLOAT *data){
    float pngvalue;
    int index;
                pngvalue = 30 * ((float)(*data)-pngmin) / (pngmax - pngmin);
                if (pngvalue > 30) pngvalue = 30;
                if (pngvalue < 0) pngvalue = 31;
                index = (int)(floorf(pngvalue));
    return (index);
 }

/* f_res_<precision> */
/* extract a binary  */
/* file into a text  */
/* file, and         */
/* optionally create */
/* a png image file  */
int main(int argc, char * argv[]){
	FILE *infile,*outfile,*geofile;
   char *line,*token,*in_fname,*out_fname,*base_fname,*label;
	int nxny,i=0,j=0;
	int finput = 0,pngflag=0,psflag=0,negflag=0,qflag=0,foutput = 0,flabel=0,cflg,errflg,ctrlflg = 0;
   int maxflag=0;
   int xsize=0,ysize=0,nc[3]; 
   extern int optind;
   extern char *optarg;
   CA_FLOAT *data,*datastart,*fsdata,*cdata,mindat,maxdat=0;
   CA_FLOAT *psdata,*psdatastart;
   float *fdata,*fdatastart;
   float pngmax,pngmin=0,pngvalue;
   int pngcolours[32];
   int index=0;
   Colours colourtype = COLOUR_REDBLUE_POS;
   gdImagePtr im_out;


    out_fname = (char *)calloc(MAX_WORD_LENGTH,sizeof(char));
    in_fname = (char *)calloc(MAX_WORD_LENGTH,sizeof(char));
    base_fname = (char *)calloc(MAX_WORD_LENGTH,sizeof(char));
    label = (char *)calloc(MAX_WORD_LENGTH,sizeof(char));
    line = (char *) calloc(MAX_STRING_LEN ,sizeof(char));
    token = (char *) calloc(MAX_STRING_LEN ,sizeof(char));
    errflg=0;

   if (argc == 1) errflg=TRUE;
   while ((cflg = getopt(argc, argv, "c:ai:o:d:hl:gPp:n:q")) != -1) {
      switch (cflg) {
      case 'a':
	      fprintf(stderr,"%s: sorry, flag 'a' is not defined\n",argv[0]);
         break;
      case 'q':
         qflag = 1;
         break;
      case 'p':
	      if (!(qflag))
            printf("Output png file\n");
	      pngflag = 1;
	      pngmax = atof(optarg);
	      if (pngmax == 0){
	         pngmax = 1;
	         maxflag = 1;
	      }
         break;
      case 'P':
	      if (!(qflag))
            printf("Output pscontour file\n");
	      psflag = 1;
         break;
      case 'n':
	      if (!(qflag))
	      printf("png - Use negative values\n");
	      negflag = 1;
	      pngmin = atof(optarg);
         break;
      case 'g':
	      if (!(qflag))
	      printf("Using control file ca_geoplus.in\n");
	      ctrlflg = 1;
         break;
      case 'c':       /* get input filename */
         colourtype=(Colours)atoi(optarg);
         break;
      case 'i':       /* get input filename */
         finput = TRUE;
         in_fname=strdup(optarg);
         debin (base_fname,in_fname);
         /*base_fname=strdup(optarg);*/
         /*sprintf(in_fname,"%s.bin",base_fname);*/
         break;
      case 'd':
         if (!(xsize = atoi(optarg))){
            fprintf(stderr,"ERROR with d argument: %s\n",optarg);
            errflg = TRUE; 
         }
         if (!( ysize = atoi(argv[optind]))){
            fprintf(stderr,"ERROR with d argument: %s\n",argv[optind]);
            errflg = TRUE; 
         }
         optind++;
         break;
      case 'o':       /* get output filename */
         foutput = TRUE;
         out_fname = strdup(optarg);
         break;
      case 'l':       /* get label string */
         flabel = TRUE;
         label = strdup(optarg);
         break;
      case 'h':
         errflg=TRUE;
         break;
      default:
         errflg=TRUE;
         break;
      }
   }
   if (errflg ) {
      fprintf(stderr,"%s -i infile  [-o outfile] "\
                     "-d xsize ysize\n"\
                      "-g to read ca_geoplus for size instead of -d\n"\
                      "-p maxval to create PNG file (0 is automatic) \n"\
                      "-P to create pscontour file\n"\
                      "-n minval to create PNG file (0 is default) \n"\
                      "-c colour type 0=redblue 1=blackwhite \n"\
                      "-l label to label the tecplot results(default is CA_RESULTS) \n"\
                      "Also, a single precision (float) binary output is produced,\n"\
                      "in <filename>_flt.bin\n"\
                      "Default out is [infile].dat\n",argv[0]);
       #ifndef ALL_VAR
       fprintf(stderr,"Recompile with -DALL_VAR to create alternate versions for including i,j\n "\
                      "this is needed for reading in more than one file in TecPlot.\n"); 
       #endif /* ALL_VAR */
      exit(0);
   }

   if (!(qflag))
       printf("Version: $Id: f_res.c 1009 2007-04-30 12:28:35Z  $\n");
   if (!foutput){
      sprintf(out_fname,"%s.dat",base_fname);
   if (!(qflag))
      printf("Default output filename used: %s\n",out_fname);
   }
   if (!finput){
      fprintf(stderr,"ERROR: no input file specified! \n");
      exit(0);
   }

   if (!flabel){
      label=strdup("CA_RESULTS");
   }
       

    if (!(qflag)){
       printf("Creating 2-d data file for TecPlot\n");
       printf("Processing CA_FLOAT results from %s into %s\n",in_fname,out_fname);
    }
    if (!(outfile = fopen(out_fname,"w"))){
      fprintf(stderr,"ERROR: could not open  outfile %s\n",out_fname);
      exit(0);
    }

    /**************************/
    if (ctrlflg){
       /* simply read the geo file for the size line only */
       if ( (geofile = fopen("ca_geoplus.in","r"))== NULL ){
          fprintf(stderr,"ERROR, could not open ca_geoplus.in\n");
          exit(0);
       }else{
          while (fgets(line,MAX_STRING_LEN,geofile)!= NULL){
               if(line[0]=='%' || line[0]=='#'||(token = strtok(line," ,;\t"))==NULL) {
               continue;
              }else if (strcasecmp(token,"NCellsPerSB") == 0){
                for (i=0; i<3; i++) {
                   if ((token = strtok(NULL, " ,;\t")) != NULL)
                      nc[i]  = atoi(token);
                   else {
                      fprintf(stderr,"Error: NCellsPerSB,not found\n");
                      exit(0);
                   }
                }
              } 
           }/*end of while loop for getting lines*/
           fclose(geofile);
           xsize = nc[0];
           ysize = nc[1];
        }
     }/* end of read geo file */
     /**************************/

	      if (!(qflag))
    printf("xsize = %i, ysize = %i\n",xsize,ysize);
    nxny = xsize * ysize;
    if (nxny <= 0) {
       fprintf(stderr,"ERROR: size is wrong: %i\n",nxny);
       fprintf(stderr,"\nSpecifiy -d xsize ysize\n");
       fprintf(stderr,"or -g to read file ca_geoplus.in \n\n");
       exit(0);
    }

   /* create a png image file */
   if (pngflag){
      if(!(qflag)){
      printf("Creating png image,\n");
      printf("Using colourtype %i\n",(int)colourtype);
      }
      /* initialize the structure */
      im_out=gdImageCreate(xsize,ysize);

      /* set up the colour table */
      #ifdef RANDCOLOUR
      for(i=0;i<32;i++){
         pngcolours[i]=gdImageColorAllocate(im_out,colours[3*i],colours[3*i+1],colours[3*i+2]);
      }
      #else
         pngcolours[0]=gdImageColorAllocate(im_out,255,255,255);
         switch (colourtype){
         case(COLOUR_BW_POS):
            pngcolours[31]=gdImageColorAllocate(im_out,255,85,85);
            for(i=1;i<31;i++){
               pngcolours[i]=gdImageColorAllocate(im_out,255 - 8*i,255-8*i,255-8*i);
            }
            break;

         case(COLOUR_REDBLUE_POS):
         default:
            for(i=1;i<32;i++){
               pngcolours[i]=gdImageColorAllocate(im_out,8*i,0,255-8*i);
            }
            break;
      }
      #endif

   }/* end Set up png objects */
      
    /* create the header info for a TecPlot data file */
    /* alternate versions for including i,j           */
    /* this is needed for reading in more than one file */
    /* into tecplot.                                    */
    #ifdef ALL_VAR
       fprintf(outfile,"VARIABLES =\"x\",\"y\", \"%s\"\n",label);
    #else
       fprintf(outfile,"VARIABLES = \"%s\"\n",label);
    #endif/*ALL_VAR*/

    #ifdef ALL_VAR
       fprintf (outfile,"ZONE I=%i,J=%i,F=POINT\n",xsize,ysize);
    #else
       fprintf (outfile,"ZONE I=%i,J=%i\n",xsize,ysize);
    #endif/*ALL_VAR*/

    /* end of set up the tecplot headers */

    
    if (!(infile = fopen(in_fname,"r"))){
      fprintf(stderr,"ERROR: could not open  infile %s\n",in_fname);
      exit(0);
    }
    /* read in the data file */
    data = datastart = (CA_FLOAT *) malloc ( nxny * sizeof(CA_FLOAT));
    fdata = fdatastart = (float *) malloc ( nxny * sizeof(float));
    if (psflag){
      psdata = psdatastart = (CA_FLOAT *) malloc ( 3*nxny * sizeof(CA_FLOAT));
    }
    fread(data,sizeof(CA_FLOAT),nxny,infile);
    fclose(infile);

    /* process the data file */
    for (j=0;j<ysize;j++){
       for(i=0;i<xsize;i++){
          /* find the scaling factor for an image to fit in the number of colours */
          if(pngflag){
             if (maxflag){
                maxdat = (maxdat > *data)? maxdat:*data;
                mindat = (mindat < *data)? mindat:*data;
             }else{
                index = scale_png(pngmin,pngmax,data);
                gdImageSetPixel(im_out,i,j,pngcolours[index]);
             }
          }
          *fdata = (float)(*data);
          /* print the data to a file */
          /* suitable for pscontour     */
          if(psflag){
             *psdata = (CA_FLOAT)(i);
             psdata++;

             *psdata = (CA_FLOAT)(j);
             psdata++;

             *psdata = *data;
             psdata++;
          }
          /* print the data to a file */
          /* suitable for TecPlot     */
          #ifdef ALL_VAR
             fprintf(outfile,"%i %i %.10g\n",i,j,*data);
          #else
             fprintf(outfile,"%.10g\n",*data);
          #endif/*ALL_VAR*/
          fdata++;
          data++;
          cdata++;
       }
    }/* end process the data file */

    /* done writing the TecPlot file */
    fclose(outfile);


    /* scale the image if autoscaleing is used */
    if(maxflag && pngflag){
    data = datastart; /*rewind*/
    pngmax = maxdat;

    if (negflag){
       pngmin = mindat;
    }else{
       pngmin = 0;
    }

       for (j=0;j<ysize;j++){
          for(i=0;i<xsize;i++){
             index = scale_png(pngmin,pngmax,data);
             gdImageSetPixel(im_out,i,j,pngcolours[index]);
             data++;
          }
       }
    }/* end autoscaling the image data */


  /* without autoscaling, the image is already scaled */
  if(pngflag){
     /*********write out the PNG output************/
      gdImageInterlace(im_out,1);
      sprintf(out_fname,"%s.png",base_fname);
      outfile=fopen(out_fname,"w");
      gdImagePng(im_out,outfile);
      fclose(outfile);
      if(!(qflag))
         printf("Created file %s\n",out_fname);
   }

   /* create a single precision binary file */
   sprintf(out_fname,"%s_flt.bin",base_fname);
   outfile=fopen(out_fname,"w");
   fwrite(fdatastart,sizeof(float),(xsize * ysize),outfile);
   fclose(outfile);

   if(psflag){
      sprintf(out_fname,"%s_psc.bin",base_fname);
      outfile=fopen(out_fname,"w");
      fwrite(psdatastart,sizeof(CA_FLOAT),3*(xsize * ysize),outfile);
      fclose(outfile);
   }

      if(!(qflag)) 
          printf("Total lines processed %i\n",i);

return(0);
}
