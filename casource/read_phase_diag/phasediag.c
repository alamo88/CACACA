/*$Id: phasediag.c 892 2006-03-10 15:24:59Z rcatwood $*/
/****************************************************************/
/*   Copyright (c) 1998 - 2004 Dept. of Materials, ICSTM        */
/*   All Rights Reserved                                        */
/*   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF ICSTM       */
/*   The copyright notice above does not evidence any           */
/*   actual or intended publication of such source code,        */
/*   and is an unpublished work by Dept. of Materials, ICSTM.   */
/*   This material contains CONFIDENTIAL INFORMATION that       */
/*   is the property of Imperial College. Any use,              */
/*   duplication or disclosure not specifically authorized      */
/*   by Imperial College is strictly prohibited.                */
/****************************************************************/
/* This code is part of the umats routines developed at in the  */
/* Materials Processing Group, Dept. of Materials, ICSTM.       */
/*      email p.d.lee or r.atwood @ic.ac.uk for details         */
/****************************************************************/
/*This file was created at Mon Jul 12 14:50:41 BST 2004 by rcatwood on hive.beowulf.cluster */
#define PHASEDIAG_C_REV "phasediag.c $Id: phasediag.c 892 2006-03-10 15:24:59Z rcatwood $"
#include <stdio.h>
#include <string.h>
#include <math.h>
/* Include the common header for this project */
#include "../machine.h"
#include "phasediag.h"
#include "csv.h"
#include "init.h"
#include "convert.h"
#include "freecsv.h"
#include "readwrite.h"
#include "ph_input.h"
#include "ph_input_struct_reader.h"
const char pd_h_rev[] = PHASEDIAG_H_REV ;
const char pd_c_rev[] = PHASEDIAG_C_REV ;

extern size_t write_phasediag(Phdiag * pdp, const char * filename);
extern size_t read_phasediag(Phdiag * pdp, const char * filename);
extern void vol_phasediag(Phdiag * pdp, const char * filename);

Pd_num_t n,liqreg,liqpha,fropha,froreg,threereg,inphases[3];


void dump_tslice(Tslice * tsp,FILE * fp){
     int i;
     fprintf(fp,"ncomp %i\n",tsp->ncomp);
     fprintf(fp,"my_temp %g\n",tsp->my_temp);
     fprintf(fp,"cmin\tcmax\tcstep\tn_csteps\tncomp %i\n",tsp->ncomp);
     for(i=0;i<tsp->ncomp;i++){ 
         fprintf(fp,"%g\t%g\t%g\t%i\n",tsp->cmin[i],tsp->cmax[i],tsp->cstep[i],tsp->n_csteps[i]);
     }
     fprintf(fp,"ndata %i\n",tsp->ndata);
     fprintf(fp,"n_alloc %i\n",tsp->n_alloc);

     /* binary for now */
     fprintf(fp,"Pd: %g ",tsp->my_temp) ;
     for(i=0;i<tsp->n_csteps[0];i++){
        fprintf(fp,"%hhi ",*(tsp->regdata  + i )) ;
     }
     fprintf(fp,"\n");
}

void dump_region(Reg * regp,FILE * fp){
    int j;
         for(j=0;j<regp->np;j++){
             fprintf(fp,"%s ",regp->phases[j].name);
         }
         fprintf(fp,"\n");
         if (regp->np ==3 ){
            fprintf(fp,"Tie Triangles:\n");
            fprintf(fp,"ntie = %i\t",regp->ntie);
            fprintf(fp,"tmin = %g\ttmax= %g\n",regp->tmin,regp->tmax);
            fprintf(fp,"Reg\tTemp\tp1c1\tp2c1\tp1c2\tp2c2\tp3c1\tp3c2\n");
            for(j=0;j<regp->ntie;j++){
               fprintf(fp,"%i\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
                     regp->tiedata->region,
                     regp->tiedata->temp,
                     regp->tiedata->p1c1,
                     regp->tiedata->p2c1,
                     regp->tiedata->p1c2,
                     regp->tiedata->p2c2,
                     regp->tiedata->p3c1,
                     regp->tiedata->p3c2);
              }
         }
         fprintf(fp,"\n");

return;
}

     
     
     
void dump_phasediag(Phdiag * pdp, FILE * fp){
      int i;
      fprintf(fp,"ncomp %i\n", pdp->ncomp);
      fprintf(fp,"tmin %g\n", pdp->tmin);
      fprintf(fp,"tmax %g\n", pdp->tmax);
      fprintf(fp,"tstep %g\n",pdp-> tstep);
      fprintf(fp,"n_tslice %i\n",pdp-> n_tslice);
      fprintf(fp,"n_alloc %i\n",pdp-> n_alloc);
      for(i=0;i<pdp->ncomp;i++){
         fprintf(fp,"comp %i num %i name %s\n",i,pdp->comp[i].my_num,pdp->comp[i].name);
      }
      for(i=0;i<pdp->n_phases;i++){
         fprintf(fp,"phase %i num %i name %s\n",i,pdp->phases[i].my_num,pdp->phases[i].name);
      }
      for(i=0;i<pdp->n_regions;i++){
         fprintf(fp,"Region %i NP %i\n",i,pdp->regions[i].np);
         dump_region(pdp->regions +i,fp);
      }

      for (i=0;i<pdp->n_tslice;i++){
          dump_tslice(pdp->tslices+i,fp);
      }
}



void init_tslice(Phdiag * pdp, Tslice * tsp,CA_FLOAT slicetemp,CA_FLOAT climits[]){
     /* climits: c_a_min, c_a_max, c_a_step, c_b_min, c_b_max, c_b_step */
     int nsteps;
     CA_FLOAT newmax;
     int i;
     tsp->ncomp = pdp->ncomp-1;
     tsp->my_temp = slicetemp;
     tsp->ndata = 1;
     for (i=0;i<2;i++){
        tsp->cmin[i] = climits[3*i];
        tsp->cmax[i] = climits[3*i+1];
        tsp->cstep[i] = climits[3*i+2];

        nsteps = (int)(FLOOR ((tsp->cmax[i] - tsp->cmin[i])/(tsp->cstep[i])));
        newmax = tsp->cmin[i] + (CA_FLOAT)(nsteps+1) * tsp->cstep[i];
        tsp->cmax[i] = newmax;
        tsp->n_csteps[i] = nsteps+1;
        tsp->ndata *= tsp->n_csteps[i];
     }

     tsp->n_alloc = tsp->ndata;
     tsp->regdata = (Pd_num_t *)calloc(tsp->n_alloc, sizeof(Pd_num_t));
     tsp->data = (CA_FLOAT **)calloc(tsp->n_alloc, sizeof(CA_FLOAT *));

}

/* allocat the space, using values placed in the *_alloc members of the structure */
void alloc_phasediag(Phdiag * pdp){
   pdp->tslices = (Tslice *) calloc (pdp->n_alloc , sizeof(Tslice));
   pdp->regions = (Reg * ) calloc(pdp->n_r_alloc,sizeof(Reg));
   pdp->phases = (Pha * ) calloc(pdp->n_p_alloc,sizeof(Pha));
   pdp->comp = (Comp *) calloc(pdp->n_c_alloc,sizeof(Comp));
}

/* Initialize a blank Phdiag structure, for use */
void init_phasediag(Phdiag * pdp){
   int nsteps;
   CA_FLOAT newmax;
   snprintf(pdp->header,255,"Phasediag info %s %s\n", pd_h_rev,pd_c_rev);
   snprintf(pdp->tailer,255,"End of phasediag info\n");
   nsteps = (int)(FLOOR ((pdp->tmax - pdp->tmin)/(pdp->tstep)));
   newmax = pdp->tmin + (CA_FLOAT)(nsteps+1) * pdp->tstep;
   pdp->tmax = newmax;
   pdp->n_tslice = nsteps+1;

   pdp->n_alloc = pdp->n_tslice;

   pdp->n_regions=0;
   pdp->n_phases=0;

   pdp->n_p_alloc = 1;
   pdp->n_r_alloc = 1;
   pdp->n_c_alloc = 1;

   alloc_phasediag(pdp);
}
   

/* set the entire isothermal section to be in a single region eg. liquid */
void set_tslice_region( Tslice * tsp,Reg region){
         memset(tsp->regdata,region.my_num,tsp->ndata);

         /* TODO: possibly not allocate the poitner **data at all in this slice? */
         /* have to make sure it is never read in that case? */

         memset(tsp->data,0,tsp->ndata * sizeof(CA_FLOAT *));

  #ifdef JUNK 
   int i;
   int j;
   for(j=0;j<tsp->ncomp;j++){
      for (i=0;i<tsp->n_csteps[j];i++){
          *(tsp->regdata + i) = region.my_num; */
         *(tsp->data + i ) = NULL;

      }
   }
   #endif
}

/* set the values for one isothermal section, given the regions and the limits */
/* below calpha the region is alpha, above cbeta the region is beta, and between the region is twoph*/

void set_tslice_ternary( Tslice * tsp,Reg * alpha,Reg *  beta,Reg * gamma, Reg *  threeph){
     

}

void set_tslice_binary( Tslice * tsp,Reg alpha,Reg twoph,Reg beta,CA_FLOAT calpha,CA_FLOAT cbeta){
   int i;
   int alpha_ph;
   int beta_ph;
   int two_ph_a;
   int two_ph_b;
   CA_FLOAT c;

   /* handle some errors */
   if (alpha.np != 1){
       fprintf(stderr,"ERROR:set_tslice_binary: alpha is not single phase.\n");
   }

   if (beta.np != 1){
       fprintf(stderr,"ERROR:set_tslice_binary: beta is not single phase.\n");
   }

   if (twoph.np != 2){
       fprintf(stderr,"ERROR:set_tslice_binary: region twoph %i is not two phase.\n",twoph.my_num);
   }

   alpha_ph = alpha.phases[0].my_num;
   beta_ph = beta.phases[0].my_num;
   two_ph_a = twoph.phases[0].my_num;
   two_ph_b = twoph.phases[1].my_num;

   if ( (alpha_ph != two_ph_a) || (beta_ph != two_ph_b) || (alpha_ph == beta_ph) || (two_ph_a == two_ph_b) ){
      fprintf(stderr,"Warning:set_tslice_binary: Phases are not well ordered.\n");
      fprintf(stderr,"A a b B: %i %i %i %i\n",alpha_ph,two_ph_a,two_ph_b,beta_ph);
   }else{
      fprintf(stdout,"A a b B: %i %i %i %i\n",alpha_ph,two_ph_a,two_ph_b,beta_ph);
   }
   


   for (i=0;i<tsp->n_csteps[0];i++){
      c =tsp->cmin[0] + i * tsp->cstep[0]  ;

      /* to be consistent, exact boundary points are given to the two phase region */
      if (c < calpha){
         /* single phase region */
         *(tsp->regdata + i) = alpha.my_num;
         *(tsp->data + i) = NULL;
      }else if(c <= cbeta){
         /* two phase region */
         *(tsp->regdata + i) = twoph.my_num;
         *(tsp->data + i) = (CA_FLOAT *)calloc(PHASEDIAG_NPERPOINT, sizeof(CA_FLOAT));


         (*(tsp->data  + i)) [P1C1] = calpha;
         (*(tsp->data  + i)) [P2C1] = cbeta;
      }else{
         /* single phase region */
         *(tsp->regdata + i) = beta.my_num;
         *(tsp->data + i) = NULL;
      }
   }
}

void get_tie_triangle(){
/* just a stub */
}

void get_point(Phdiag * pdp,Tslice * tsp, int index,Pd_point * result){
    CA_FLOAT ** datap;
    Pd_num_t * regp;
      datap=tsp->data + index; 
      regp=tsp->regdata + index;

      result->region = *regp;

      switch (pdp->regions[*regp].np){
          case 0:
             fprintf(stderr,"ERROR:get_point: cannot have 0 phases! \n");
             exit(0);
          break;
          case 1:
          break;
          case 2:
            result->p1c1 = (*(datap))[P1C1];
            result->p1c2 = (*(datap))[P1C2];
            result->p2c1 = (*(datap))[P2C1];
            result->p2c2 = (*(datap))[P2C2];
          break;
          case 3:
            get_tie_triangle();
          break;
          default:
             fprintf(stderr,"WARNING:get_point: cannot handle %i phases! \n",pdp->regions[*regp].np);
             exit(0);
          break;
       }
}




void probe_ternary_tslice(Phdiag * pdp,Tslice * tsp,  CA_FLOAT conc[2],Pd_point * result){
    int concstep[2];
    int index;
    int i;
    Pd_point * resultp;


   for (i=0;i<2;i++){
      if(conc[i] > tsp->cmax[i] || conc[i] < tsp->cmin[i]){
         fprintf(stderr,"ERROR:probe_ternary_tslice: concentration %i outside of limits %g\n",i,conc[i]);
         exit(0);
      }
      concstep[i] = (int)(FLOOR( (conc[i]-tsp->cmin[i]) /(tsp->cstep[i]) ));
   }

      /* increase the conc1 by one step */
      resultp = result;
      index = concstep[0] + concstep[1]*tsp->n_csteps[0];
      resultp->conc1 = tsp->cmin[0] + concstep[0] * tsp->cstep[0];
      resultp->conc2 = tsp->cmin[1] + concstep[1] * tsp->cstep[1];
      get_point(pdp,tsp,index,resultp);

      /* increase the conc1 by one step */
      resultp++;
      index = (1+concstep[0]) + concstep[1]*tsp->n_csteps[0];
      resultp->conc1 = tsp->cmin[0] + (1+concstep[0]) * tsp->cstep[0];
      resultp->conc2 = tsp->cmin[1] + concstep[1] * tsp->cstep[1];
      get_point(pdp,tsp,index,resultp);

      /* increase the conc2 by one step */
      resultp++;
      index = (concstep[0]) + (1+concstep[1])*tsp->n_csteps[0];
      resultp->conc1 = tsp->cmin[0] + concstep[0] * tsp->cstep[0];
      resultp->conc2 = tsp->cmin[1] + (1+concstep[1]) * tsp->cstep[1];
      get_point(pdp,tsp,index,resultp);

      /* increase both conc1 and conc2 by one step */
      resultp++;
      index = (1+concstep[0]) + (1+concstep[1])*tsp->n_csteps[0];
      resultp->conc1 = tsp->cmin[0] + (1+concstep[0]) * tsp->cstep[0];
      resultp->conc2 = tsp->cmin[1] + (1+concstep[1]) * tsp->cstep[1];
      get_point(pdp,tsp,index,resultp);

   return;
}

Pd_point * probe_binary_tslice(Phdiag * pdp,Tslice * tsp,  CA_FLOAT conc){
    int concstep;
    Pd_point * result;
    CA_FLOAT ** datap;
    Pd_num_t * regp;

    if(conc > tsp->cmax[0] || conc < tsp->cmin[0]){
      fprintf(stderr,"ERROR:probe_binary_tslice: concentration outside of limits %g\n",conc);
      exit(0);
   }
   concstep = (int)(FLOOR( (conc-tsp->cmin[0]) /(tsp->cstep[0]) ));
   result = (Pd_point *) calloc(2 , sizeof(Pd_point));

      datap=tsp->data + concstep;
      regp=tsp->regdata + concstep;

      result->region = *regp;

      if (pdp->regions[*regp].np == 2 ){
         result->p1c1 = (*(datap))[P1C1];
         result->p1c2 = (*(datap))[P1C2];
         result->p2c1 = (*(datap))[P2C1];
         result->p2c2 = (*(datap))[P2C2];
      }

      result->conc1 = tsp->cmin[0] + concstep * tsp->cstep[0];


      datap=tsp->data   + concstep+1;
      regp=tsp->regdata + concstep+1;

      (result+1)->region = *regp;
      if (pdp->regions[*regp].np == 2 ){
         (result+1)->p1c1 = (*(datap))[P1C1];
         (result+1)->p1c2 = (*(datap))[P1C2];
         (result+1)->p2c1 = (*(datap))[P2C1];
         (result+1)->p2c2 = (*(datap))[P2C2];
      }
      (result+1)->conc1 = tsp->cmin[0] + (concstep+1) * tsp->cstep[0];

   return(result);
}

void probe_ternary(Phdiag * pdp, CA_FLOAT temp, CA_FLOAT conc[2],Pd_point * result){
   int tempstep;
   int i;
   Pd_point *ures,*lres;

   if(temp > pdp->tmax || temp < pdp->tmin ){
      fprintf(stderr,"ERROR:probe_binary: temperature outside of limits %g\n",temp);
      exit(0);
   }

   tempstep = (int)(FLOOR ( temp - pdp->tmin)/(pdp->tstep));

   /* get the upper four points */
   lres =  result;
   probe_ternary_tslice(pdp,pdp->tslices+tempstep,conc,lres);
   for (i=0;i<4;i++){
      lres[i].temp = pdp->tslices[tempstep].my_temp;
   }

   /* get the lower four points */
   ures = result + 4;
   probe_ternary_tslice(pdp,pdp->tslices+tempstep+1,conc,ures);

   for (i=0;i<4;i++){
      ures[i].temp = pdp->tslices[tempstep+1].my_temp;
   }

   return;
 }

Pd_point * probe_binary(Phdiag * pdp, CA_FLOAT temp, CA_FLOAT conc){
   int tempstep;
   Pd_point * result,*ures,*lres;

   if(temp > pdp->tmax || temp < pdp->tmin ){
      fprintf(stderr,"ERROR:probe_binary: temperature outside of limits %g\n",temp);
      exit(0);
   }

   result = (Pd_point *) calloc(4 , sizeof(Pd_point));

   tempstep = (int)(FLOOR ( temp - pdp->tmin)/(pdp->tstep));
   lres = probe_binary_tslice(pdp,pdp->tslices+tempstep,conc);
   lres[0].temp = pdp->tslices[tempstep].my_temp;
   lres[1].temp = pdp->tslices[tempstep].my_temp;

   ures = probe_binary_tslice(pdp,pdp->tslices+tempstep+1,conc);

   ures[0].temp = pdp->tslices[tempstep+1].my_temp;
   ures[1].temp = pdp->tslices[tempstep+1].my_temp;

   memcpy(result,lres,2*sizeof(Pd_point));
   memcpy(result+2,ures,2*sizeof(Pd_point));


   free (lres);
   free (ures);
   return(result);
 }


/* add a new phase, returning the id number of that phase */
/* expanding the phase array if necessary */
Pd_num_t addphase(Phdiag * pdp,char * name){
    int newphase;

    newphase = pdp->n_phases;
    if (newphase > MAX_PD_NUM){
        fprintf(stderr,"ERROR:addphase: maximum number of phases %i exceeded. %i/n",(MAX_PD_NUM),newphase);
        exit(0);
    }
    if (newphase >= pdp->n_p_alloc){
        pdp->n_p_alloc += PHASEDIAG_ADD ;
        pdp->phases = (Pha *) realloc(pdp->phases,pdp->n_p_alloc * sizeof (Pha));
    }


    pdp->phases[newphase].my_num = (Pd_num_t)(newphase);
    snprintf(pdp->phases[newphase].name,MAX_NAME_LEN,name);
    pdp->n_phases ++;
    return( pdp->phases[newphase].my_num );
}

Pd_num_t addcomp(Phdiag * pdp,char * name){
    int newcomp;

    newcomp = pdp->ncomp;
    if (newcomp > MAX_PD_NUM){
        fprintf(stderr,"ERROR:addcomp: maximum number of comps %i exceeded. %i/n",(MAX_PD_NUM),newcomp);
        exit(0);
    }
    if (newcomp >= pdp->n_c_alloc){
        pdp->n_c_alloc += PHASEDIAG_ADD ;
        pdp->comp = (Comp *) realloc(pdp->comp,pdp->n_c_alloc * sizeof (Comp));
    }


    pdp->comp[newcomp].my_num = (Pd_num_t)(newcomp);
    snprintf(pdp->comp[newcomp].name,MAX_NAME_LEN,name);
    pdp->ncomp ++;
    return( pdp->comp[newcomp].my_num );
}

Pd_num_t addregion(Phdiag * pdp,int np, Pd_num_t inphases[]){
   int newreg;
   int i;

   newreg = pdp->n_regions;
    if (newreg > MAX_PD_NUM){
        fprintf(stderr,"ERROR:addregion: maximum number of regions %i exceeded. %i/n",(MAX_PD_NUM),newreg);
        exit(0);
    }
    if (newreg >= pdp->n_r_alloc){
        pdp->n_r_alloc += PHASEDIAG_ADD ;
        pdp->regions = (Reg *) realloc(pdp->regions,pdp->n_r_alloc * sizeof (Reg));
    }
    pdp->regions[newreg].my_num = (Pd_num_t)(newreg);
    pdp->regions[newreg].np = np;
    pdp->regions[newreg].phases = (Pha *) calloc(np,sizeof(Pha));
    pdp->regions[newreg].ntie = 0;/* initialize */
    pdp->regions[newreg].tmin = 0;
    pdp->regions[newreg].tmax = 0;
    pdp->regions[newreg].tiedata = (Pd_tri *)(NULL);
    /* must subsequently call set_threephase to allocate this space */

    for(i=0;i<np;i++){
       if ( (int)(inphases[i]) >= pdp->n_phases ){
           fprintf(stderr,"ERROR:addregion: Input phase is out of range %i\n",(int)(inphases[i]));
           exit(0);
       }

       pdp->regions[newreg].phases[i] = pdp->phases[inphases[i]];

    }
    pdp->n_regions++;
    return(pdp->regions[newreg].my_num) ;
}

void alloc_tiedata(Reg * regp){
    regp->tiedata = (Pd_tri *) calloc(regp->ntie,sizeof(Pd_tri));
}

void set_threephase(Phdiag * pdp,Reg * regp,CA_FLOAT tmin,CA_FLOAT tmax){
    int minstep,maxstep;
    minstep = (int)(floor((tmin - pdp->tmin)/pdp->tstep));
    maxstep = (int)( ceil((tmax - pdp->tmin)/pdp->tstep));
    minstep = MAX(0,minstep);
    maxstep = MIN(pdp->n_tslice,maxstep);

    regp->tmin = (float)(minstep) * pdp->tstep + (float)pdp->tmin;
    regp->tmax = (float)(maxstep) * pdp->tstep + (float)pdp->tmin;

    regp->ntie = maxstep - minstep;
    alloc_tiedata(regp);
}

/* subroutine to set the data as input from the .dat file, this routine may need to be modified */
/* to account for differences in data files */

void set_tslice_data(Phdiag * pdp, Tslice * tsp, CsvFloat * the_float,int tstep){
    int i,j;
    CA_FLOAT ** datap,*dp;
    Pd_num_t reg;

    datap = the_float->data + tstep * tsp->ndata;
    for(i=0;i<tsp->n_csteps[0];i++){
       for(j=0;j<tsp->n_csteps[1];j++){
          if ( (*datap)[0] != tsp->my_temp){
             fprintf(stderr,"WARNING: set_tslice_data: temp does not match %g %g \n", (*datap)[0],tsp->my_temp);
          }

          /* Honbiao Dong;s data file includes '100' for the concentration if a tie line is not valid */
          /* it is not clear whether this is liquid or not */

          /* New data form -- includes region number in the last column */
          /* hope it is consistent with the input file */

          if (the_float->nfields <= 8 ){
             fprintf(stderr,"ERROR: region data not found in column 8 \n In fact, column 8 was not found at all! \n");
             exit(0);
          }
          
          reg = (Pd_num_t) ((*datap)[8]);
          *(tsp->regdata + j* tsp->n_csteps[0] + i) = reg;

          switch (pdp->regions[reg].np){
             case 0:
                fprintf(stderr,"ERROR:set_tslice_data: region %i cannot have 0 phases! \n",reg);
             break;
             case 1:
             break;
             case 2:
                dp = *(tsp->data + j* tsp->n_csteps[0] + i) = (CA_FLOAT *)calloc(PHASEDIAG_NPERPOINT, sizeof(CA_FLOAT));
                dp[P1C1] = (*datap)[5];
                dp[P1C2] = (*datap)[6];
                dp[P2C1] = (*datap)[3];
                dp[P2C2] = (*datap)[4];
             break;
             case 3:
             break;
             default:
                fprintf(stderr,"ERROR:set_tslice_data: region %i cannot have %i phases! \n",reg,pdp->regions[reg].np);
             break;
          }
       datap++;
       }/* end of j loop */
    }/* end of i loop */
return;
}

void debug_view_tslice(Tslice * tsp){
   Pd_num_t testview[30][15];
   int i,j;
    for(i=0;i<tsp->n_csteps[0];i++){
       for(j=0;j<tsp->n_csteps[1];j++){
          testview[i][j] =    *(tsp->regdata + j* tsp->n_csteps[0] + i) ;
       }/* end of j loop */
    }/* end of i loop */
 }

      
void testprobe(Phdiag * pdp, CA_FLOAT testT, CA_FLOAT testC1, CA_FLOAT testC2){

     Pd_num_t rg,rgz,numreg=0,treg=1;
     int i,k;
     static Pd_point probed[8];
     CA_FLOAT testconc[2];
     Pd_num_t * countreg;
     testconc[0] = testC1;
     testconc[1] = testC2;
     countreg = (Pd_num_t *) calloc(pdp->n_regions,sizeof(Pd_num_t));

     probe_ternary(pdp,testT,testconc,probed);

      printf("Result of probing at %g %g %g\n",testT, testC1,testC2);
      printf("Point\tT\tC1\tC2\tRegion\tNphases\tPhase1(id:name)\tPhase2(id:name)\n");
           printf("\n");
      rgz=probed[0].region;
      for(i=0;i<8;i++){
      
           rg=probed[i].region;
           treg = (treg && (rgz == rg));
           if (countreg[rg] == 0){
              numreg ++;
              countreg[rg] = 1; 
           }


           printf("%i\t%g\t%g\t%g\t%i\t%i\t",i,probed[i].temp,probed[i].conc1,probed[i].conc2,rg,pdp->regions[rg].np);

           for (k=0;k<pdp->regions[rg].np;k++){
               printf("%i:%s\t",k,pdp->regions[rg].phases[k].name);
           }

           printf("\n");
           if (pdp->regions[rg].np == 2){
              printf("%i tie line:\n\tp1c1\t%g\n\tp1c2\t%g\n\tp2c1\t%g\n\tp2c2\t%g\n",i,probed[i].p1c1, probed[i].p1c2,probed[i].p2c1,probed[i].p2c2);
           }
      }

      printf("Found %hhi regions\n",numreg);
      if (treg){
         printf("Regions are all the same\n");
      }else{
         printf("Regions are different\n");
      }
      printf("treg :&& %hhi\t",treg);
      printf("\n\n");
      free(countreg);
   }
#ifdef TESTMAIN

int main (int argc, char * argv[]){
   int i;
   int nwrite=0;
   int startstep;
   int endstep;
   PhInp input;
   Phdiag * pdp;
   CsvData * the_data;
   CsvFloat * the_float;
   CA_FLOAT slicetemp;

   pdp = (Phdiag *) calloc (1, sizeof(Phdiag));
   input_ph_input_struct_values(argv[1],&input);
   pdp->tmin = input.Tmin;
   pdp->tmax = input.Tmax;
   pdp->tstep = input.Tstep;
   init_phasediag(pdp);
   addphase(pdp,"Zerophase");
   for (i=0;i<input.nphases;i++){
     n = addphase(pdp,input.phase[i]);
     if (strcasecmp("liquid",input.phase[i])==0) liqpha = n;
   }
   for (i=0;i<input.ncomp;i++){
     n = addcomp(pdp,input.components[i]);
   }
   /* create a special phase to indicate temperature below appicable range */
   fropha = addphase(pdp,"FROZEN");

   inphases[0] = 0;
   addregion(pdp,1,inphases);/* zero region */

   for (i=0;i<input.nregions;i++){
      n = addregion(pdp,(*(input.region +i))[1],(*(input.region+i)+2));
      if (pdp->regions[n].phases[0].my_num == liqpha && pdp->regions[n].np == 1) liqreg = n;
      if ( (*(input.region+i))[1] == 3 ) threereg = n;
   }
   inphases[0] = fropha;
   /* special region below applicable range */
   froreg = addregion(pdp,1,inphases);

   set_threephase(pdp,(pdp->regions+threereg),775,785);

   the_data = (CsvData *)calloc(1,sizeof(CsvData));
   the_float = (CsvFloat *)calloc(1,sizeof(CsvFloat));
   the_data->nheaders = 1;
   init_csv_data(the_data);
   read_csv(input.filename,the_data);
   convert_csv(the_float, the_data);

   startstep = (int)(floor(input.Tstart - pdp->tmin)/pdp->tstep);
   endstep = (int)(floor(input.Tend - pdp->tmin)/pdp->tstep);

   /* initialize all the slices */
   for (i=0;i<pdp->n_tslice;i++){
      slicetemp = i * pdp->tstep + pdp->tmin;
      init_tslice(pdp,(pdp->tslices+i),slicetemp,input.Climits);
   }

   /* set the slices below the range as FROZEN */
   for(i=0;i<startstep;i++){
      slicetemp = pdp->tslices[i].my_temp;
      fprintf(stdout,"Setting frozen: %g\n",slicetemp);
         set_tslice_region( (pdp->tslices) + i,pdp->regions[froreg]);
   }
   /* set the slices above the range as LIQUID */
   for(i=endstep;i<pdp->n_tslice;i++){
      slicetemp = pdp->tslices[i].my_temp;
      fprintf(stdout,"Setting liquid: %g\n",slicetemp);
         set_tslice_region( (pdp->tslices) + i,pdp->regions[liqreg]);
   }
   for (i=startstep;i<endstep;i++){
      slicetemp = pdp->tslices[i].my_temp;
      fprintf(stdout,"Inputting %g\n",slicetemp);
      set_tslice_data( pdp,(pdp->tslices+i),the_float,(i-startstep));
   }
      

   /* write out the phase diagram in binary data form */

   nwrite = write_phasediag(pdp,"phase");

   for (i=0;i<pdp->n_tslice;i++){
     debug_view_tslice(pdp->tslices+i);
   }
   vol_phasediag(pdp,"testphase");
   fprintf(stdout,"Wrote %i in write_phasediag\n",(int)nwrite);

   testprobe(pdp,775.6,0.2225,0.1123);

  free_input(&input);
  free_csv_data(the_data);
  free(the_data);
  free_float_data(the_float);
  free(the_float);
return(0);
}
#endif
#ifdef TESTMAINORIG
int main (int argc, char * argv[]){
   int i,j,k;
   size_t nwrite;
   Pd_num_t lnum,anum,bnum, inphases[3];
   Pd_num_t lreg,areg,breg,lareg,lbreg,abreg,abcreg;
   Pd_num_t * countreg;

   CA_FLOAT testT, testC;
   CA_FLOAT T_pure=600,m_alloy=-12,k_alloy=0.1;
   CA_FLOAT T_eut=525,C_eut;
   CA_FLOAT climits[6],slicetemp ;
   CA_FLOAT csol,cliq,clim ;
   Phdiag * pdp,*pdp2;
   Pd_point * probed;
   C_eut = (T_eut - T_pure)/m_alloy;

   fprintf(stdout,"C_eut = %g\n",C_eut);

   pdp = (Phdiag *) calloc (1, sizeof(Phdiag));
   pdp2 = (Phdiag *) calloc (1, sizeof(Phdiag));

   pdp->ncomp = 3;
   pdp->tmin = 520;
   pdp->tstep = 1;
   pdp->tmax = 620;

   climits[0]=0;
   climits[1]=10;
   climits[2]=0.1;

   climits[3]=1;
   climits[4]=12;
   climits[5]=1;



   
   init_phasediag(pdp);


   lnum = addphase(pdp,"Liquid");
   anum = addphase(pdp,"Alpha");
   bnum = addphase(pdp,"Beta");

   inphases[0] = lnum;
   lreg = addregion(pdp,1,inphases);

   inphases[1] = anum;
   lareg = addregion(pdp,2,inphases);

   inphases[0] = anum;
   areg = addregion(pdp,1,inphases);

   inphases[0] = lnum;
   inphases[1] = bnum;
   lbreg = addregion(pdp,2,inphases);

   inphases[0] = bnum;
   breg = addregion(pdp,1,inphases);

   inphases[0] = anum;
   inphases[1] = bnum;
   abreg = addregion(pdp,2,inphases);

   inphases[2] = lnum;

   abcreg = addregion(pdp,3,inphases);
   set_threephase(pdp,(pdp->regions+abcreg),520,550);
   
   for (i=0;i<pdp->n_tslice;i++){
      slicetemp = i * pdp->tstep + pdp->tmin;
      init_tslice(pdp,(pdp->tslices+i),slicetemp,climits);
   }

   for(i=0;i<pdp->n_tslice;i++){
      slicetemp = pdp->tslices[i].my_temp;
      fprintf(stdout,"Inputting %g\n",slicetemp);
      if (slicetemp > T_pure){
         set_tslice_region( (pdp->tslices) + i,pdp->regions[lreg]);
      }else if (slicetemp > T_eut) {
         cliq = (slicetemp-T_pure)/m_alloy;
         csol = k_alloy * cliq;
         set_tslice_binary( (pdp->tslices) + i,pdp->regions[areg],
                                               pdp->regions[lareg],
                                               pdp->regions[lreg],
                                               csol,cliq);
      }else{
         clim = k_alloy * C_eut;
         set_tslice_binary((pdp->tslices) + i,pdp->regions[areg],
                                              pdp->regions[abreg],
                                              pdp->regions[breg],
                                              clim,35);
      }
   }

   set_tslice_ternary( pdp->tslices + 5,pdp->regions+areg,
                                        pdp->regions+breg,
                                        pdp->regions+lreg,
                                        pdp->regions+abcreg);

   dump_phasediag(pdp,stdout);

   testT=599.5;
   testC=0.55;

   
   for(j=0;j<50;j++){

     Pd_num_t rg,rgz,numreg=0;
     Pha * ph;
     Pd_num_t treg = 1;
     countreg = (Pd_num_t *) calloc(pdp->n_regions,sizeof(Pd_num_t));

      testC=0.1*j;
      probed = probe_binary(pdp,testT,testC);

      printf("Result of probing at %g %g\n",testT, testC);
      printf("Point\tT\tC\tRegion\tNphases\t");
           printf("\n");
      rgz=probed[0].region;
      for(i=0;i<4;i++){
      
           rg=probed[i].region;
           treg = (treg && (rgz == rg));
           if (countreg[rg] == 0){
              numreg ++;
              countreg[rg] = 1; 
           }


           printf("%i\t%g\t%g\t%i\t%i\t",i,probed[i].temp,probed[i].conc1,rg,pdp->regions[rg].np);

           for (k=0;k<pdp->regions[rg].np;k++){
               printf("%i:%s ",k,pdp->regions[rg].phases[k].name);
           }

           printf("\n");
      }

      printf("Found %hhi regions\n",numreg);
      if (treg){
         printf("Regions are all the same\n");
      }else{
         printf("Regions are different\n");
      }
      printf("treg :&& %hhi\t",treg);
      free(countreg);
   }
   
   nwrite = write_phasediag(pdp,"phase");
   vol_phasediag(pdp,"testphase");
   fprintf(stdout,"Wrote %i in write_phasediag\n",(int)nwrite);


return(0);
}
#endif /*TESTMAIN*/

#ifdef READMAIN

int main (int argc, char * argv[]){
   Phdiag * pdp,*pdp2;
   pdp = (Phdiag *) calloc (1, sizeof(Phdiag));
   pdp2 = (Phdiag *) calloc (1, sizeof(Phdiag));

   #ifdef BL_COMPRESS
   read_phasediag(pdp2,"phase.pdz");
   #else
   read_phasediag(pdp2,"phase.pdg");
   #endif
   dump_phasediag(pdp2,stdout);
   vol_phasediag(pdp2,"readtest");
return(0);
}
#endif /* READMAIN */
