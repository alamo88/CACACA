/*RCS Id:$Id: readwrite_fg.c 887 2006-03-01 18:21:01Z rcatwood $*/
#include <stdio.h>
#include <string.h>
#include "safeopen.h"
#include "../machine.h"
#include "../constants.h"
#include "../fidap.h"
#include "read_fg_list.h"
#include "readwrite_fg.h"


/****************************/
/* write the FGrid_str data */
/* to a csv text file       */
/****************************/
void write_fg_csv(const char * outfilename,FGrid_str *fg){
   int i,j;
   CA_FLOAT * tp;
   FILE * ofile;
   int nr = fg->nr,nz=fg->nz;
   ofile = safeopen(outfilename,"w");

   fprintf(ofile,"nz,nr,nnodes\n%i,%i,%i\n",fg->nz,fg->nr,fg->nnodes);

   fprintf(ofile,"r values,");
   for (i=0;i<nr-1;i++){
      fprintf(ofile,"%.10g,",fg->r[i]);
   }
   fprintf(ofile,"%.10g\n",fg->r[i]);

   fprintf(ofile,"z values,");
   for (i=0;i<nz-1;i++){
      fprintf(ofile,"%.10g,",fg->z[i]);
   }
   fprintf(ofile,"%.10g\n",fg->z[i]);


   fprintf(ofile,"T values\n");

  tp = fg->Fidap_T;
  for (i=0;i<nr;i++){
     fprintf(ofile,"%.10g,",fg->r[i]);
     for (j=0;j<nz;j++){
         fprintf(ofile,"%.10g,",*tp++);
      }
      fprintf(ofile,"\n");
  }
  fclose(ofile);
}

/****************************/
/* write the FGrid_str data */
/* to a binary   file       */
/****************************/
/******************************************************************/
/* File structure:                                                */
/* CR terminated string giving the data structure revision number */
/*                                                                */
/* one int (4 B) giving the size (in bytes) of a CA_FLOAT variable   */
/*                                                                */
/* one int (4 B) giving the size (in bytes) of the structure      */
/*                                                                */
/* the data in the structure itself = number of bytes as given    */
/*                                                                */
/* the data arrays for r, z, then T; number of bytes determined    */
/* from CA_FLOAT size and the nr, nz, and nnodes values stored in    */
/* the data structure                                             */
/*                                                                */
/******************************************************************/
void write_fg_bin(const char * outfilename,FGrid_str *fg){
 const char frev[] = FG_REV;
 int fsize,fgsize,revsize;
 long data_pos=0,info_data_pos; /* position of the data, pointer to the location for that information */
 FILE * ofile;

 fsize = sizeof(CA_FLOAT);
 fgsize = sizeof(FGrid_str);
 revsize = strlen(frev);

 ofile = safeopen(outfilename,"w");
 /* save the revision ID string */
 fwrite(frev,sizeof(char),revsize,ofile);
 fwrite("\n",sizeof(char),1,ofile);

 /* save the compiled-in size of a CA_FLOAT variable */
 fwrite(&fsize,sizeof(int),1,ofile);

 /* get the current location, to put the data_pos into*/
 /* and make space for it */
 info_data_pos = ftell(ofile);
 fwrite(&data_pos,sizeof(long),1,ofile);


 /* save the current size of a FGrid_str structure */
 fwrite(&fgsize,sizeof(int),1,ofile);

 /* save the structure data */
 fwrite(fg,fgsize,1,ofile);

 /* save the data arrays */
 /* get the position of the begginning of the arrays */
 data_pos=ftell(ofile);
 fwrite(fg->r,sizeof(CA_FLOAT),fg->nr,ofile);
 fwrite(fg->z,sizeof(CA_FLOAT),fg->nz,ofile);
 fwrite(fg->Fidap_T,sizeof(CA_FLOAT),fg->nnodes,ofile);
 /* store the beginning position in the file */
 fseek(ofile,info_data_pos,SEEK_SET);
 fwrite(&data_pos,sizeof(long),1,ofile);

 fclose(ofile);
}

/**********************************************/
/* read in the binary data for a fg structure */
/* adn check the data for consistancy         */
/* read only the r,z,and T data               */
/**********************************************/

/* print error message and exit */
void err_ex(void){
   fprintf(stderr,"ERROR: malloc failed\n");
   exit(0);
}


/* allocate the arrays pointed to by the fg sructure*/
void alloc_fg(FGrid_str *fg){
    fg->r = (CA_FLOAT *) malloc(fg->nr * sizeof(CA_FLOAT));
       if (fg->r == NULL) err_ex();
    fg->z = (CA_FLOAT *) malloc(fg->nz * sizeof(CA_FLOAT));
       if (fg->r == NULL) err_ex();
    fg->Fidap_T = (CA_FLOAT *) malloc(fg->nnodes * sizeof(CA_FLOAT));
       if (fg->r == NULL) err_ex();
}

/* read data for the arrays pointed to by the fg sructure */
void read_fg_data(FGrid_str *fg, FILE * ifile){
    int nread=0;
    nread = fread(fg->r,sizeof(CA_FLOAT),fg->nr,ifile);
    fprintf(stderr,"Read %i CA_FLOAT for r data\n",nread);
    nread = fread(fg->z,sizeof(CA_FLOAT),fg->nz,ifile);
    fprintf(stderr,"Read %i CA_FLOAT for z data\n",nread);
    nread = fread(fg->Fidap_T,sizeof(CA_FLOAT),fg->nnodes,ifile);
    fprintf(stderr,"Read %i CA_FLOAT for T data\n",nread);
}

/*********************************************************************/
/*                                                                   */
/*subroutine to read the data into the fg structure, including tests */
/* for the version of the data structure                             */
/*                                                                   */
/*********************************************************************/
void read_fg_bin(const char * infilename,FGrid_str *fg,int fg_flag){
    int fsize,fgsize;
    int nread;
    FILE * ifile;
    const char frev[] = FG_REV;
    char line[LINELENGTH];
    long data_pos=0,data_compare=0;

    fprintf(stderr,"read_fg_bin: Reading from %s\n",infilename);
    /* open the file */
    ifile = safeopen(infilename,"r");
    if (fgets(line,LINELENGTH,ifile) == NULL){
       fprintf(stderr,"read_fg_bin:ERROR: Cannot read a line\n");
       #ifdef ERROR_EXIT
       exit(1);
       #endif
    }
    line[strlen(line)-1]=(char)0;
    /*test the revision numbers*/
    /* test the cvs revision number of fidap.h */
    fprintf(stderr,"Revision of fidap.h ... \nThis program %s\nBin file %s\n",frev,line);
    if (strcasecmp(line,frev) != 0){
       fprintf(stderr,"**************************************************\n");
       fprintf(stderr,"* read_fg_bin: WARNING: structure revision numbers do not match.\n");
       fprintf(stderr,"* In the file: %s, In this program: %s\n",line,frev);
       fprintf(stderr,"* Attempting to continue...\n");
       fprintf(stderr,"* If there are more errors below, the structure revisions\n");
       fprintf(stderr,"* are probably not compatible.\n");
       fprintf(stderr,"* In this case you should checkout the correct\n");
       fprintf(stderr,"* older version of the ca program and use that,\n");
       fprintf(stderr,"* or use (or create) a filter version to convert the data.\n");
       fprintf(stderr,"**************************************************\n");
    }
    /* test the size of CA_FLOAT - compiled precision */
    fread (&fsize,sizeof(int),1,ifile);
         fprintf(stderr,"CA_FLOAT size: file: %i program: %i\n",fsize,sizeof(CA_FLOAT));
    if(fsize != sizeof(CA_FLOAT)){
         fprintf(stderr,"ERROR:CA_FLOAT precision does not match.");
         fprintf(stderr,"ERROR: Cannot continue in this case.");
         exit(0);
    }
    /* locate the numerical data section */
    fread(&data_pos,sizeof(long),1,ifile);
    fprintf(stderr,"Location of data: %i\n",data_pos);



    switch (fg_flag){
       case FG_FIRST_READ:
       case FG_TRANS_READ:
          /* test the size of fg struct */
          fread (&fgsize,sizeof(int),1,ifile);
             fprintf(stderr,"fg struct size: %i %i\n",fgsize,sizeof(FGrid_str));
          if(fgsize != sizeof(FGrid_str)){
             fprintf(stderr,"ERROR: FGrid_str size does not match.");
             fprintf(stderr,"ERROR: Cannot continue in this case.");
             exit(0);
          }
          /* if we made it this far, read in the FGrid_str data */
          nread=fread(fg,fgsize,1,ifile);
          fprintf(stderr,"Read %i bytes for fg struct\n",nread*fgsize);
          fprintf(stderr,"nr = %i,nz = %i,nnodes = %i\n",fg->nr,fg->nz,fg->nnodes);

          /* allocate the arrays */
          alloc_fg(fg); /* this routine exits on malloc failure */
          break;
      case 500:/* this method may not work very well....*/
      
          /* set the file pointer to the location of the data */
          fseek(ifile, data_pos,SEEK_SET);
          break;
       default:
          fprintf(stderr,"ERROR:read_fg_bin: Flag not understood %i\n",fg_flag);
          exit(0);
       break;
    }


    /* read in the data values */
    read_fg_data(fg,ifile);
    fclose (ifile);
 }/* end of read_fg_bin */


    
/*
RCS Log:$Log$
RCS Log:Revision 11.1  2006/03/01 18:21:00  rcatwood
RCS Log:Merging polycomponent and gas with meltback
RCS Log:
RCS Log:Revision 10.2  2005/12/01 14:38:02  rcatwood
RCS Log:Merged xly_05 changes into the main trunk
RCS Log:Primarily involving melt-back
RCS Log:
RCS Log:Revision 10.1.2.2  2005/11/23 18:19:10  rcatwood
RCS Log:Result of merging mould_source and xly meltback+curvature 2d versions
RCS Log:
RCS Log:Revision 10.1  2005/11/03 11:56:48  rcatwood
RCS Log:New version number -- using mould_src as base
RCS Log:
RCS Log:Revision 1.1.4.2  2005/11/02 11:50:56  rcatwood
RCS Log:Read list files
RCS Log:
RCS Log:Revision 9.2  2003/12/10 17:36:23  kerman
RCS Log:added unit conversions & fixed for counting number of fields from the header
RCS Log:
RCS Log:Revision 1.1.2.5  2003/03/12 18:32:32  rcatwood
RCS Log:Added diagnostic output to readwrite_fg
RCS Log:
RCS Log:Revision 1.1.2.4  2003/02/27 23:04:50  rcatwood
RCS Log:Removed use of old temperature routines , all temperatures shoudl
RCS Log:be determined by checking the array c_temp in teh subblock, if the
RCS Log:subblock is open
RCS Log:
RCS Log:Revision 1.1.2.3  2003/02/25 19:43:58  kerman
RCS Log:change the unit system from BS to SI as it's necessary for decentred square
RCS Log:
RCS Log:Revision 2.3.4.2  2003/01/22 16:52:15  rcatwood
RCS Log:Changed the sort order to conform with xuehua and weishan input routines.
RCS Log:
RCS Log:Revision 2.3.4.1  2003/01/15 19:44:40  rcatwood
RCS Log:*** empty log message ***
RCS Log:
RCS Log:Revision 2.3  2002/12/13 18:54:35  rcatwood
RCS Log:adjusted Gmakefile to include the subdirectory of read_csv routines
RCS Log:
RCS Log:Revision 2.2  2002/12/13 17:04:25  rcatwood
RCS Log:Changed to a part of ca source treeh
RCS Log:
RCS Log:Revision 2.1  2002/10/17 16:56:01  rcatwood
RCS Log:*** empty log message ***
RCS Log:
RCS Log:Revision 1.2  2002/09/06 13:06:42  rcatwood
RCS Log:improved for compatiblity with CA code
RCS Log:
RCS Log:Revision 1.1  2002/09/06 12:52:36  rcatwood
RCS Log:Cured all memory allocation problems .. I hope.
RCS Log:
RCS Log:Revision 1.2  2002/09/05 18:05:44  rcatwood
RCS Log:Included convert, read and write fg binary, and tested. It seems to work.
RCS Log:
RCS Log:Revision 1.1  2002/09/04 14:58:33  rcatwood
RCS Log:First working version -- reads and writes CSV files (no conversion)
RCS Log:
*/
