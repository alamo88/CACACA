
/****************************************************************/
/*      Copyright (c) 1993 Peter D Lee                          */
/*      Copyright (c) 1998 Dept. of Materials, ICSTM            */
/*      All Rights Reserved                                     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code,     */
/*      and is an unpublished work by Dept. of Materials, ICSTM.*/
/*      continuing D Phil work from University of Oxford        */
/****************************************************************/
/* This code is part of the umats routines developed at in the  */
/* Materials Processing Group, Dept. of Materials, ICSTM.       */
/*      email p.d.lee or r.atwood @imperial.ac.uk for details   */
/****************************************************************/

/********************************************************************************/
/*  This version is distributed under a BSD style public license, as follows:   */
/*                                                                              */
/*  Copyright (c) 2007, Dept. of Materials, Imperial College London             */
/*  All rights reserved.                                                        */
/*  Redistribution and use in source and binary forms, with or without          */
/*  modification, are permitted provided that the following conditions          */
/*  are met:                                                                    */
/*                                                                              */
/*  * Redistributions of source code must retain the above copyright            */
/*  notice, this list of conditions and the following disclaimer.               */
/*                                                                              */
/*  * Redistributions in binary form must reproduce the above                   */
/*  copyright notice, this list of conditions and the following                 */
/*  disclaimer in the documentation and/or other materials provided             */
/*  with the distribution.                                                      */
/*                                                                              */
/*  * Neither the name of the Dept. of Materials, Imperial College London, nor  */
/*  the names of its contributors may be used to endorse or promote products    */
/*  derived from this software without specific prior written permission.       */
/*                                                                              */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS         */
/*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT           */
/*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR       */
/*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT        */
/*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       */
/*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    */
/*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      */
/*  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      */
/*  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        */
/*  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          */
/*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                */
/********************************************************************************/
/*END of LICENSE NOTICE*/

/****************************************************************/
/* sb_nuc.c:                                          */
/* Calculate the new number of nuclei formed and place them     */
/* in cells in the current subblock.                            */
/****************************************************************/
/****************************************************************/
/* Written by Peter D. Lee & Robert C. Atwood, Imperial College */
/* Jul 1, 1998                                                  */
/****************************************************************/
/*       MODIFIED by:                                    */
/*  PDL: Aug 22, 1998                                    */
/****************************************************************/
/****** To Do List **********************************************/
/*General:                                          */
/* 1) Fix serious bug in rappaz nucleation that causes t dep.   */
/* 1) Add heterogenous nucleation                               */
/****************************************************************/
/*RCS Id:$Id: sb_nuc.c 1398 2008-11-13 11:37:26Z  $*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef EXTERNAL_CA
   #include "common.h"
#endif

#include "machine.h"
#include "blocks.h"
#include "umat_histo.h"
extern void sb_get_surface_cells (BB_struct * bp, int sbnum);
extern int init_nuc_mould (BB_struct * bp, int sbnum, FILE * listfile);

/********************************************************/
/* THRESH_MESS macro - put a message for each threshold */
/* uses preprocessor macros to tell user which fucntion */
/* was called when the message was printed.             */
/********************************************************/

#ifndef THRESH_MESS
#define THRESH_MESS if (nuc_flag == 1){ \
       if (nucmsg < MAX_NUC_MSG){\
          fprintf(stderr,""THISFUNC": Nucleating grain, threshold  = %.5g, T=  %f, o_T = %f\n",threshold, Tunder,old_Tunder);\
          nucmsg++;\
       }\
   } else {\
       if (nonucmsg < MAX_NUC_MSG){\
          fprintf(stderr,""THISFUNC":NOT Nucleating grain, threshold  = %.5g, T=  %f, o_T = %f\n",threshold, Tunder,old_Tunder);\
          nonucmsg++;\
       }\
   }

#endif /*THRESH_MESS */
/*******************************************/
/* end of THRESH_MESS macro                */
/*******************************************/

/* functions defined later in current file */
int calc_num_nuc_rappaz (BB_struct * bp, int snum);
int calc_num_nuc_het (BB_struct * bp, int snum);
int init_new_grain (BB_struct * bp, int igr, int sbnum, int xcell, int ycell, int zcell, int nc);
int init_fixed_angle_grain (BB_struct * bp, int igr, int sbnum, int icell, int nc, CA_FLOAT a0, CA_FLOAT a1, CA_FLOAT a2);
int nuchisto (FILE * listfile, BB_struct * bp, CA_FLOAT * thr);

/* used on ly for RECRYSTALLIZE*/
int calc_num_nuc_recr (BB_struct * bp, int snum);

/*function from recr_extra.c*/
extern CA_FLOAT nuc_rate_recr (CA_FLOAT nuc_c, CA_FLOAT temp, CA_FLOAT lnZ, CA_FLOAT eps);

/****************************************************************/
/****************************************************************/
/* sb_nuc: Calculate the new number of nuclei formed and        */
/* place them in cells in the current subblock.                 */
/****************************************************************/
/****************************************************************/
int sb_nuc (BB_struct * bp, int sbnum)
{
  int i, j, index, recr_index;  /* tmp counters */
  int Xindex, Yindex, Zindex;
  int nnuclei;                  /* number of new nuclei formed... */
  CA_FLOAT tmp;                 /* tmp CA_FLOAT var. */
  CA_FLOAT oct_fact = bp->oct_factor;   /* tmp CA_FLOAT var. */
  CA_FLOAT dtmp0, dtmp1, dtmp2;
  double dtmp;                  /* tmp CA_FLOAT var. */
  SB_struct *sp;
  Ind_grain *g;

/************************************************/
/* Check if in test mode of FIXED_NUC!!!        */
/************************************************/
  if (bp->ctrl->fixed_nuc) {
    if (bp->step == INIT) {     /* set the nuclei */
      Ctrl_str *cp;

      cp = bp->ctrl;

      /*for (i=0; i<cp->nfnuc; i++) { */
      for (i = 0; i < 0; i++) { /* need change Wei WANG 30-01-03 */
        index = (int) (cp->nsite[i][0] + cp->nsite[i][1] * bp->nc[0]
                       + cp->nsite[i][2] * bp->nc[0] * bp->nc[1]);

        if (index > bp->ncsb) {
          fprintf (stderr, "ERROR: fixed-nuc outside of subblock,%i\n", index);
          fprintf (stderr, "     : setting to zero.\n");
          index = 0;
        }

        bp->sb[sbnum]->c_fs[index] = EMBRYO;
        bp->nprops.ngr += 1;
        bp->sb[sbnum]->ngr += 1;
        bp->sb[sbnum]->gr[index] = bp->nprops.ngr;

        /* init the grain structure for that grain... */
        dtmp0 = PI_BY_FOUR / 45.0 * cp->nsite[i][3];
        dtmp1 = PI_BY_FOUR / 45.0 * cp->nsite[i][4];
        dtmp2 = PI_BY_FOUR / 45.0 * cp->nsite[i][5];
        init_fixed_angle_grain (bp, bp->nprops.ngr, sbnum, index, 1, dtmp0, dtmp1, dtmp2);      /*by Wei WANG 30-07-02 */

        g = bp->gr[bp->nprops.ngr];

        /* set all of them to 1.0 and then only switch the ones required! */
        for (j = 0; j < bp->ctrl->n_neigh; j++) {
          g->mdir[j] = 1.0;
        }
/* Problem : this causes floating point errors in tan function */
/** \todo  fix floating point errors, replace completely with WEI routine?*/
#ifdef JUNK
#ifdef NASTAC_RULE
        g->dir_fact = 1 / (tan (fabs (dtmp)));
        g->dir_angle = dtmp;
        g->gro_fact = 1.0;
        if (g->dir_fact < 2.0) {
          g->ph = 0.0;
          g->pq = 1.0;
          g->po = 0.0;
        } else if (g->dir_fact < 4.0) {
          g->ph = 1.0;
          g->pq = 0.0;
          g->po = 0.0;
        } else {
          g->ph = 0.0;
          g->pq = 0.0;
          g->po = 1.0;
        }
#else /* RCA_RULE */
        g->dir_fact = fabs (tan (dtmp));
        g->dir_angle = dtmp;
        g->ph = 1.0 - (g->dir_fact);
        g->pq = (1 - oct_fact) * (1.0 - g->ph);
        g->po = oct_fact * (1.0 - g->ph);
        g->gro_fact = 1 - 0.5 * cos (dtmp);
        /*
           g->gro_fact =  4*( (g->ph)/6 + (g->pq)/4 + (g->po)/8);
         */
#endif /*Nastac rule */
#endif   /*JUNK*/
          /* g->mdir[7] = cos( dtmp ); */
          /* g->mdir[1] = cos( dtmp ) * cos( dtmp ); */
          fprintf (stderr, "sb_nuc: nucleating fixed gr[%d] at cell %d.\n", bp->nprops.ngr, index);
      }
    }
    return (0);
  }

/************************************************/
/* Get number of nuclei formed...               */
/************************************************/
  if (bp->nprops.nmodel == N_HETERO) {
    nnuclei = calc_num_nuc_het (bp, sbnum);
  } else {
    nnuclei = calc_num_nuc_rappaz (bp, sbnum);
  }
  if (nnuclei < 0) {
#ifdef VERBOSE_NUC_MSG
    fprintf (stderr, "SB[%d] is too hot for nuclei to form.\n", sbnum);
#endif /*VERBOSE_NUC_MSG */
  } else if (nnuclei == 0) {
#ifdef VERBOSE_NUC_MSG
    fprintf (stderr, "SB[%d] has no new nuclei.\n", sbnum);
#endif /*VERBOSE_NUC_MSG */
/********************************************************/
/* New nuclei have formed, add them into cells randomly */
/********************************************************/

  } else {                      /* (nnuclei > 0) */
#ifdef VERBOSE_NUC_MSG
    fprintf (stderr, "SB[%d] adding %d new nuclei.\n", sbnum, nnuclei);
#endif /*VERBOSE_NUC_MSG */
    for (i = 0; i < nnuclei; i++) {
      index = (int) (drand48 () * bp->ncsb);
      index = MIN (index, (bp->ncsb - 1));
      Zindex = (int) ((CA_FLOAT) index / (bp->nc[0] * bp->nc[1]));
      Yindex = (int) (((float) index - (float) Zindex * (bp->nc[0] * bp->nc[1])) / (bp->nc[0]));
      Xindex = (int) (index - Zindex * (bp->nc[0] * bp->nc[1]) - Yindex * (bp->nc[0]));
      /* if (bp->sb[sbnum]->c_fs[index] == LIQUID ) { *//*taken out by Wei WANG on 20-09-02 */
      if (bp->sb[sbnum]->gr[index] == 0) {      /*added by Wei WANG on 20-09-02 */
        if (bp->nprops.ngr == bp->nprops.gd_max_total) {
          fprintf (stderr, "ERROR: sb_nuc - Max # grains set by user [%d] exceeded. Increase option MaxTotGrains.\n",
                   bp->nprops.gd_max_total);
        } else {
          bp->sb[sbnum]->c_fs[index] = EMBRYO;
          bp->nprops.ngr += 1;
          bp->sb[sbnum]->ngr += 1;
          bp->sb[sbnum]->gr[index] = bp->nprops.ngr;
          /* init the grain structure for that grain... */
          init_new_grain (bp, bp->nprops.ngr, sbnum, Xindex, Yindex, Zindex, 1);
          bp->gr[bp->nprops.ngr]->TNuc = bp->sb[sbnum]->Tvals.Tavg;
          bp->gr[bp->nprops.ngr]->TunderNuc = bp->mprops.Tliq - bp->gr[bp->nprops.ngr]->TNuc;
          bp->gr[bp->nprops.ngr]->CellConcNuc = 0;
          bp->gr[bp->nprops.ngr]->max[0] = Xindex;
          bp->gr[bp->nprops.ngr]->max[1] = Yindex;
          bp->gr[bp->nprops.ngr]->max[2] = Zindex;
          bp->gr[bp->nprops.ngr]->min[0] = Xindex;
          bp->gr[bp->nprops.ngr]->min[1] = Yindex;
          bp->gr[bp->nprops.ngr]->min[2] = Zindex;
        }
      }
    }
  }
  return (0);
}                               /* end of sb_nuc subroutine */

/****************************************************************/
/****************************************************************/
/* Subroutine to calculate the number of NEW nuclei formed      */
/* using the gaussian distribution of RAPPAZ.                   */
/* by the successive integration method (doesnt work well)      */
/****************************************************************/
/****************************************************************/
int calc_num_nuc_rappaz (BB_struct * bp, int sbnum)
{
  int i, j;                     /* tmp counters */
  int new_nuclei;               /* number of new nuclei */
  CA_FLOAT Tliq, tau, tauold, Tavgold, Tavg, tmin, Tnuc, Tunder;        /*tau is distance from gauss. ctr */
  double tmp1, tmp2;            /* tmp double var. */
  Nuc_str *np;
  SB_nuc_str *snp;

/* set all pointers to point to structure values  */
  np = &(bp->nprops);
  /* SB nucleation stuff */
  snp = &(bp->sb[sbnum]->sbnuc);        /* Subblock Nucleation Pointer snp */
  /* cell and grain arrays  */

/* ohter stuff -- not using pointers  */
  Tliq = bp->mprops.Tliq;
  Tavg = bp->sb[sbnum]->Tvals.Tavg;
  Tavgold = bp->sb[sbnum]->Tvals.Tavg_old;
  tmin = bp->sb[sbnum]->Tvals.TminReached;

/* determine the new total number of nuclei as a fn of under-cooling */
  Tnuc = Tliq - (np->NucParams[0]);
  Tunder = Tliq - Tavg;
  tau = Tavg - Tnuc;
  tauold = Tavgold - Tnuc;
  tmp2 = 0;

  if ((Tunder) < 0) {           /* return if no undercooling */
    return (-1);
  } else if (tmin < Tavg) {     /* return if reached max undercooling */
    return (-2);
  } else {
    /* calculate total # grains... */
    tmp1 = 1.0 / (SQRT2 * np->NucParams[1]);
    /* the density of new nuclei - per m^3 */
    tmp1 = 0.5 * np->gd_max * (erf ((double) tauold * tmp1) - erf ((double) tau * tmp1));

    snp->N_nuc_old = snp->N_nuc;
    snp->N_nuc = tmp1;          /* store for output */
    tmp2 = tmp1 * bp->vol_c * bp->ncsb; /* # in vol. sb */
    tmp2 += (double) snp->fract_nuc;
    new_nuclei = floor (tmp2);  /* integer number formed */
    snp->fract_nuc = (CA_FLOAT) (tmp2 - (CA_FLOAT) new_nuclei); /* fraction residual */
    return (new_nuclei);
  }
}                               /* end of calc_num_nuc_rappaz subroutine */

int calc_num_nuc_recr (BB_struct * bp, int sbnum)
{
  int i, j;                     /* tmp counters */
  int new_nuclei;               /* number of new nuclei */
  CA_FLOAT Temp, Nc, Eps, lnZ;
  double tmp1, tmp2;            /* tmp double var. */
  Nuc_str *np;
  SB_nuc_str *snp;

/* set all pointers to point to structure values  */
  np = &(bp->nprops);
  /* SB nucleation stuff */
  snp = &(bp->sb[sbnum]->sbnuc);        /* Subblock Nucleation Pointer snp */

/* ohter stuff -- not using pointers  */
  Temp = bp->Tinit;
  Nc = np->gd_max;              /*kludge for recry. */
  Eps = np->NucParams[0];
  lnZ = bp->mprops.surf_tens;

/* determine the new total number of nuclei as a fn of under-cooling */
  tmp2 = 0;

  /* the density of new nuclei - per mm^3 */
  tmp1 = nuc_rate_recr (Nc, Temp, lnZ, Eps);

  snp->N_nuc_old = snp->N_nuc;
  snp->N_nuc = tmp1;            /* store for output */
  tmp2 = tmp1 * bp->vol_c * bp->ncsb;   /* # in vol. sb */
  tmp2 += (double) snp->fract_nuc;
  new_nuclei = floor (tmp2);    /* integer number formed */
  snp->fract_nuc = (CA_FLOAT) (tmp2 - (CA_FLOAT) new_nuclei);   /* fraction residual */
  return (new_nuclei);

}                               /* end of calc_num_nuc_recry subroutine */

   /*tmp1=nuc_rate_recr(np->gd_max,bp->Tinit,bp->mprops.surf_tens,np->NucParams[0],); */

/****************************************************************/
/****************************************************************/
/* Subroutine to calculate the number of NEW nuclei formed      */
/* using the HETEROGENIOUS NUCLEATION MODEL.                    */
/****************************************************************/
/****************************************************************/
int calc_num_nuc_het (BB_struct * bp, int sbnum)
{
  int i, j;                     /* tmp counters */
  int sum_new_nuclei, new_nuclei;       /* number of new nuclei */
  CA_FLOAT Tliq, Tavg, tmin, Tnuc, Tunder;
  double dtmp;
  CA_FLOAT tmp;
  Nuc_str *np;
  Nucleus_str *nuc;
  SB_nuc_str *snp;

/* set all pointers to point to structure values  */
  np = &(bp->nprops);
  /* SB nucleation stuff */
  snp = &(bp->sb[sbnum]->sbnuc);        /* Subblock Nucleation Pointer snp */
  /* cell and grain arrays  */

/* ohter stuff -- not using pointers  */
  Tliq = bp->mprops.Tliq;
  Tavg = bp->sb[sbnum]->Tvals.Tavg;
  tmin = bp->sb[sbnum]->Tvals.TminReached;

/* determine the new total number of nuclei as a fn of under-cooling */
  Tnuc = Tliq - (np->NucParams[0]);
  Tunder = Tliq - Tavg;
#ifdef VERBOSE_NUC_MSG
  printf ("calc_num_nuc_het: Tliq[%f], Tavg[%f], tmin[%f], Tnuc[%f], Tunder[%f]\n", Tliq, Tavg, tmin, Tnuc, Tunder);
#endif /* VERBOSE_NUC_MSG */

  if ((Tliq - Tavg) < 0) {      /* return if no undercooling */
    return (-1);
  } else if (tmin < Tavg) {     /* return if reached max undercooling */
    return (-2);
  } else {                      /* calculate total # grains... */
#ifdef VERBOSE_NUC_MSG
    printf ("calc_num_nuc_het: nhet: %d\n", np->nhet);
    for (i = 0; i < np->nhet; i++)
      printf ("\t het[%d]: Nmax,A,rad,b %g %f %f %f\n", i, np->nuc[i].Nmax, np->nuc[i].A, np->nuc[i].rad, np->nuc[i].B);
#endif /* VERBOSE_NUC_MSG */

    sum_new_nuclei = 0;

    /* Loop through the number of species */
    for (i = 0; i < np->nhet; i++) {
      nuc = &(np->nuc[i]);
      /* check if there are any nuclei left ... */
      if (!nuc->fin) {
        /* the density of new nuclei - per mm^3 */
            /*** OLD VERSION tmp = - (nuc->B * nuc->ftheta ) / ( (Tunder*Tunder) ); new_nuclei = nuc->A * exp(dtmp); */
        dtmp = -(nuc->B) / ((Tunder * Tunder));
        new_nuclei = nuc->Nmax * nuc->CA_A * nuc->rad * nuc->rad * exp (dtmp);
        nuc->Nrate = new_nuclei;
        new_nuclei = bp->delt * new_nuclei;

        /* Cap the max. # of nuclei */
        if ((nuc->Ntot + new_nuclei) >= nuc->Nmax) {
          new_nuclei = nuc->Nmax - nuc->Ntot;
          nuc->Ntot = nuc->Nmax;
          nuc->fin = TRUE;
        } else {
          nuc->Ntot += new_nuclei;
        }
        sum_new_nuclei += new_nuclei;   /* add in the fraction x # */
      }
    }

    snp->N_nuc_old = snp->N_nuc;
    snp->N_nuc = sum_new_nuclei;        /* store for output */
    tmp = sum_new_nuclei * bp->vol_c * bp->ncsb;        /* # in vol. sb */
    tmp += (double) snp->fract_nuc;
    new_nuclei = floor (tmp);   /* integer number formed */
    snp->fract_nuc = (CA_FLOAT) (tmp - (CA_FLOAT) new_nuclei);  /* fraction residual */

#ifdef VERBOSE_NUC_MSG
    printf ("calc_num_nuc_het: sum_new_nuclei, tmp: %d, %f\n", sum_new_nuclei, tmp);
    printf ("calc_num_nuc_het: new %d, fract %f\n", new_nuclei, snp->fract_nuc);
#endif /* VERBOSE_NUC_MSG */
    return (new_nuclei);
  }

}                               /* end of calc_sb subroutine */

/****************************************************************/
/****************************************************************/
/* Subroutine to initiate the grain structure for a new grain   */
/****************************************************************/
/****************************************************************/
int init_new_grain (BB_struct * bp, int igr, int sbnum, int xcell, int ycell, int zcell, int nc)
{
  int i, j;                     /* tmp counters */
  int icell;
  double dtmp,rnd1,rnd2,rnd3;
  CA_FLOAT tmp;
  CA_FLOAT a0, a1, a2;
  CA_FLOAT c0, s0, c1, s1, c2, s2;
  CA_FLOAT oct_fact = bp->oct_factor;
  Ind_grain *g;

/* note that later it might be better to malloc and destroy the */
/* grain structures after finished growing and only keep array  */
/* of pointers... */
  if (!(g = (Ind_grain *) malloc (sizeof (Ind_grain)))) {
    fprintf (stderr, "ERROR: Ind_grain malloc failed\n");
    return (2);
  }
  icell = xcell + bp->nc[0] * ycell + bp->nc[0] * bp->nc[1] * zcell;

  bp->gr[igr] = g;
  /* now initialise the grain structure */
  g->num = igr;
  g->cell = icell;
  g->state = 1;
  g->sbnum = sbnum;
  g->ncells = nc;
  g->ngrow = nc;
  g->blocks = 1;                /* starts off occupying only one subblock */
  /* set the extent of the grain */
  g->max[0] = xcell;
  g->max[1] = ycell;
  g->max[2] = zcell;

  g->min[0] = xcell;
  g->min[1] = ycell;
  g->min[2] = zcell;

  g->nuccell[0] = xcell;
  g->nuccell[1] = ycell;
  g->nuccell[2] = zcell;
/* add in an orientation factor if required. Call drand48 either way! */
  /*dtmp = PI_BY_FOUR * drand48(); */
  rnd1=drand48();
  dtmp = PI_BY_FOUR - 2 * (PI_BY_FOUR * rnd1 );
  if (bp->nprops.oriented) {
    for (j = 0; j < bp->ctrl->n_neigh; j++) {
      g->mdir[j] = 1.0;
    }
#ifdef NASTAC_RULE
    g->dir_fact = 1 / (tan (fabs (dtmp)));
    g->dir_angle = dtmp;
    g->gro_fact = 1.0;
    if (g->dir_fact < 2.0) {
      g->ph = 0.0;
      g->pq = 1.0;
      g->po = 0.0;
    } else if (g->dir_fact < 4.0) {
      g->ph = 1.0;
      g->pq = 0.0;
      g->po = 0.0;
    } else {
      g->ph = 0.0;
      g->pq = 0.0;
      g->po = 1.0;
    }
#else /* RCA_RULE */
    g->dir_fact = fabs (tan (dtmp));
    g->dir_angle = dtmp;
    g->ph = 1.0 - (g->dir_fact);
    g->pq = (1 - oct_fact) * (1.0 - g->ph);
    g->po = oct_fact * (1.0 - g->ph);
    g->gro_fact = 1 - 0.5 * cos (dtmp);
#endif /*Nastac Rule */
    /*
       g->gro_fact =  4.0*( (g->ph)/6.0 + (g->pq)/4.0 + (g->po)/8.0);
     */
  } else {
    g->dir_fact = 1.0;
    g->dir_angle = dtmp;
  }

  /* generate rotate matrix for grain */
  /* added by Wei WANG on 20-09-02 */
  /**todo Generate uniformly oriented grains in random nuclation*/
  if (bp->ctrl->decentred_octahedron == TRUE) {
     switch(bp->ctrl->random_angles){
	case (0): 
    a0 = 0.0;                  /* need to be changed */
    a1 =  0.0;
    a2 = 0.0;
    break;
        default: /* fallthrough, to case 1 */
	case(1): /* orientation varied in the plane */
    a0= rnd1 * DEG43 + DEG1 ; /* limit to 1->44 degreees */
    // a0 = dtmp;                  /* original code */
    a1 =  0.0;
    a2 = 0.0;
    break;
       case(2): /* uniform variation in 3.d space */
       rnd2=drand48();
       rnd3=drand48();
       a0= rnd1 * DEG43 + DEG1 ;
       dtmp = rnd2 * SIN44M1 + SIN1 ;
       a1 = asin(dtmp) ;
       a2= rnd3 * DEG43 + DEG1 ;

       
    break;
    }

    g->ang[0] = a0;
    g->ang[1] = a1;
    g->ang[2] = a2;

    c0 = g->cang[0] = cos (g->ang[0]);
    s0 = g->sang[0] = sin (g->ang[0]);
    c1 = g->cang[1] = cos (g->ang[1]);
    s1 = g->sang[1] = sin (g->ang[1]);
    c2 = g->cang[2] = cos (g->ang[2]);
    s2 = g->sang[2] = sin (g->ang[2]);

    /* rotation matrix */
    g->g[0][0] = c2 * c0 - s2 * s1 * s0;
    g->g[1][0] = c2 * s0 + s2 * c1 * c0;
    g->g[2][0] = s2 * s1;

    g->g[0][1] = -1. * s2 * c0 - c2 * c1 * s0;
    g->g[1][1] = -1. * s2 * s0 + c2 * c1 * c0;
    g->g[2][1] = c2 * s1;

    g->g[0][2] = s1 * s0;
    g->g[1][2] = -1. * s1 * c0;
    g->g[2][2] = c1;
  }

  return 0;
}                               /* end of init_new_grain subroutine */

/****************************************************************/
/* Subroutine to initiate the grain structure for a new grain   */
/* by Wei Wang on 30-07-02                                      */
/****************************************************************/
/****************************************************************/
int init_fixed_angle_grain (BB_struct * bp, int igr, int sbnum, int icell, int nc, CA_FLOAT a0, CA_FLOAT a1, CA_FLOAT a2)
{
  CA_FLOAT dtmp;
  CA_FLOAT c0, s0, c1, s1, c2, s2;
  Ind_grain *g;

/* note that later it might be better to malloc and destroy the */
/* grain structures after finished growing and only keep array  */
/* of pointers... */
  if (!(g = (Ind_grain *) malloc (sizeof (Ind_grain)))) {
    fprintf (stderr, "ERROR: Ind_grain malloc failed\n");
    return (2);
  }
  fprintf (stderr, "init_fixed_angle_grain: creating fixed grain at %i, angle %.5g %.5g %.5g\n", icell, a0, a1, a2);

  bp->gr[igr] = g;
  /* now initialise the grain structure */
  g->num = igr;
  g->cell = icell;
  g->sbnum = sbnum;
  g->ncells = nc;

  /* given angle */

  g->ang[0] = a0;
  g->ang[1] = a1;
  g->ang[2] = a2;

  c0 = g->cang[0] = cos (g->ang[0]);
  s0 = g->sang[0] = sin (g->ang[0]);
  c1 = g->cang[1] = cos (g->ang[1]);
  s1 = g->sang[1] = sin (g->ang[1]);
  c2 = g->cang[2] = cos (g->ang[2]);
  s2 = g->sang[2] = sin (g->ang[2]);

  /* rotation matrix */
  g->g[0][0] = c2 * c0 - s2 * s1 * s0;
  g->g[1][0] = c2 * s0 + s2 * c1 * c0;
  g->g[2][0] = s2 * s1;

  g->g[0][1] = -1. * s2 * c0 - c2 * c1 * s0;
  g->g[1][1] = -1. * s2 * s0 + c2 * c1 * c0;
  g->g[2][1] = c2 * s1;

  g->g[0][2] = s1 * s0;
  g->g[1][2] = -1. * s1 * c0;
  g->g[2][2] = c1;

  return 0;
}                               /* end of init_fixed_angle_grain subroutine */

/*******************************************/
/* Init nuc threshold - create a threshold for each cell in the block */
/* for use with block_cell_nuc method   BLOCK   */
/*******************************************/
int init_nuc_thresh (BB_struct * bp, int sbnum)
{
  int errors = 0;
  int i, index;
  int nx, ny, nxny;
  int n_fixed_nuc, n_nuc, newnuc;
  int n_created = 0;
  CA_FLOAT dtmp0, dtmp1, dtmp2;
  CA_FLOAT *thr, threshold;
  SB_struct *sp;
  Ctrl_str *cp;
  FILE *listfile;
  char filename[MAX_STRING_LEN];
  SurCell *surface;
  int Cell[3];

  sprintf (filename, "G_L_sb%i_%s.csv", sbnum, bp->ctrl->fn_base);
  listfile = fopen (filename, "w+");

  sp = bp->sb[sbnum];
  cp = bp->ctrl;
  nx = bp->nc[0];
  ny = bp->nc[1];
  nxny = nx * ny;

  thr = sp->c_nuc_thresh;
  for (i = 0; i < bp->ncsb; i++) {
    *(thr + i) = NUC_THRESH_NONE;
  }
  fprintf (listfile, "Grain List\n");
  fprintf (listfile, "mean,stdev,Par2,Par3\n");
  for (i = 0; i < 4; i++)
    fprintf (listfile, "%.5g,", bp->nprops.NucParams[i]);
  fprintf (listfile, "\n");

  fprintf (listfile, "Cellnum,Thresh\n");
  /* pointer is messed up?? */
  if (thr == NULL) {
    fprintf (stderr, "ERROR: init_nuc_thresh: aarrgghh!\nDebug!\n");
    errors++;
    exit (1);
  }

  /* figrue out how many random nuclei */
  n_nuc = (int) (bp->nprops.gd_max * bp->vol_sb);

  /* add fixed nuclei *//* Wei WANG 30-01-03 */
  n_fixed_nuc = cp->nfnuc;
  n_nuc += n_fixed_nuc;

  /* no nuclei??? */
  if (n_nuc < 1) {
    fprintf (stderr, "ERROR: init_nuc_thresh: no nuclei! creating one ...\n");
#      ifdef ERROR_EXIT
    fprintf (stderr, "... but ERROR_EXIT is defined, so the program is exiting.\n");
    exit (1);
#      endif /*ERROR_EXIT */
    errors++;
    n_nuc = 1;
  }

  /* more nuclei than cells???  */
  if (n_nuc > bp->ncsb) {
    fprintf (stderr, "WARNING: init_nuc_thresh: too many nuclei! Resetting ...\n");
    n_nuc = bp->ncsb;
  }

  /* create the nuclei */
  fprintf (stderr, "init_nuc_thresh: subblock %i -- Number of nuclei will be: %i\n", sbnum, n_nuc);

  /* create pre-fixed nuclei threshold *//*Wei WANG 30-01-03 */
  for (i = 0; i < n_fixed_nuc; i++) {
    index = (int) (cp->nsite[i][0] + cp->nsite[i][1] * nx + cp->nsite[i][2] * nxny);
    fprintf (stderr, "sb_nuc: adding fixed nuc at %i : threshold %.5g\n", index, cp->nsite[i][6]);
    if (index > bp->ncsb) {
      fprintf (stderr, "ERROR: fixed-nuc outside of subblock,%i\n", index);
      fprintf (stderr, "     : setting to zero.\n");
      index = 0;
    }
    *(thr + index) = cp->nsite[i][6];
  }

  /* create random nuclei threshold */
  for (i = n_fixed_nuc; i < n_nuc; i++) {
    /* find a new site that is not taken */
    do {
      int tries = 0;

      newnuc = (int) FLOOR (drand48 () * bp->ncsb);
      if (tries++ > bp->ncsb) {
        fprintf (stderr, "ERROR: init_nuc_thresh: Double Aaargh!\nYou have to debug.\n");
        errors++;
        exit (1);
      }
/* changed to allow threshold=0 */
    } while (*(thr + newnuc) >= 0);

      /****************************************************************************/
    /* find a gaussian devated, or other function, that is within #defined range */
    /* paranoia check that it is greater than zero also! */
    /* but G_NUC_MIN_UND ought to be >= 0 anyways */
      /****************************************************************************/
    do {
      int tries = 0;

      /* random function set up on readmat.c as an option */
      /* using a pointer to one of the available random functions */
      threshold = (*(bp->nprops.rand_function)) (bp->nprops.NucParams);
      if (tries++ > bp->ncsb) {
        fprintf (stderr, "ERROR: init_nuc_thresh: Could not find a nucleation site after %i tries.\n", tries);
        fprintf (stderr, "ERROR: init_nuc_thresh: Triple Aaargh!\nYou have to debug.\n");
        errors++;
        exit (1);
      }
    } while ((ABS (threshold - bp->nprops.NucParams[0]) > G_NUC_SIG_MULT * bp->nprops.NucParams[1]) || (threshold < G_NUC_MIN_UND)
             || (threshold < 0));
    n_created++;
    *(thr + newnuc) = threshold;
#ifdef LIST_ALL_NUC
     getxyz (newnuc, bp->nc, Cell);
    fprintf (listfile, "%i,%i,%i,%.5g\n", Cell[0],Cell[1],Cell[2], threshold);
#endif /*LIST_ALL_NUC */
  }


  /* nucleate at mould bdy */
  if (bp->ctrl->mould_nuc) {
    errors += init_nuc_mould (bp, sbnum, listfile);
  }
#  ifdef HISTO_NUC
  errors += nuchisto (listfile, bp, sp->c_nuc_thresh);
#  endif /*HISTO_NUC */
  fclose (listfile);
  return (errors);

}                               /* end of init_nuc_threshold */





/**********************************************************************/
/* Init nuc threshold - create a threshold for each cell in the block */
/* for use with block_cell_nuc method   BLOCK                         */
/* Extension to a multiphase system                                   */
/* by THUINET 03/05                                                   */
/**********************************************************************/
int init_nuc_thresh_poly(BB_struct *bp,int sbnum){
# define NMAX 10
   int errors = 0;
   int i, index;
   int nx, ny, nxny;
   int n_fixed_nuc, n_nuc,newnuc;
   int n_created = 0;
/*THUINET 05_05*/
   int iphs,iphs_tot;
   int int1,int2;
   int n_nuc_poly[NMAX],n_fixed_nuc_poly[NMAX];
   int *nat_site;
/*FIN THUINET 05_05*/
/*THUINET 10/05*/
   int inuc_law,inuc_law_tot;
/*END THUINET 10/05*/
   CA_FLOAT dtmp0, dtmp1, dtmp2;
   CA_FLOAT *thr,threshold;
   SB_struct *sp;
   Ctrl_str *cp;
   FILE * listfile;
   char  filename[MAX_STRING_LEN];
   SurCell * surface;
   int Cell[3];

   sprintf(filename,"G_L_sb%i_%s.csv",sbnum,bp->ctrl->fn_base);
   listfile = fopen(filename,"w");

   sp = bp->sb[sbnum];
   cp = bp->ctrl;
   nx = bp->nc[0];
   ny = bp->nc[1];
   nxny = nx*ny;

   thr=sp->c_nuc_thresh;
   nat_site=sp->nat_sol_site;

#     ifndef NUC_THRESH_NONE
#        define NUC_THRESH_NONE (-1.0)
#     endif /*NUC_THRESH_NONE*/

# define NAT_SOL_NONE (-1)
   for (i = 0; i < bp->ncsb; i++){
     *(thr + i) = NUC_THRESH_NONE;
     *(nat_site + i) = NAT_SOL_NONE;
   }
   fprintf(listfile,"Grain List\n");
   fprintf(listfile,"mean,stdev,Par2,Par3\n");
   for(i=0;i<4;i++)
      fprintf(listfile,"%.5g,",bp->nprops.NucParams[i]);
   fprintf(listfile,"\n");

   fprintf(listfile,"Cellnum,Thresh\n");
   /* pointer is messed up?? */
   if (thr == NULL){
      fprintf(stderr,"ERROR: init_nuc_thresh: aarrgghh!\nDebug!\n");
      errors++;
      exit(1);
   }


   n_nuc = 0;

   for (inuc_law = 0; inuc_law<bp->ctrl->NUM_NUC_LAW; inuc_law++) {

     /* figure out how many random nuclei */
     n_nuc_poly[inuc_law] = (int)( bp->nprops.gd_max_poly[inuc_law] * bp->vol_sb);
     n_nuc += n_nuc_poly[inuc_law];

   }

   /* add fixed nuclei */
   n_fixed_nuc = cp->nfnuc;
   n_nuc += n_fixed_nuc;

   /* no nuclei???*/
   if (n_nuc < 1 ){
     fprintf(stderr,"ERROR: init_nuc_thresh: no nuclei for %d! creating one ...\n",inuc_law);
#    ifdef ERROR_EXIT
     fprintf(stderr,"... but ERROR_EXIT is defined, so the program is exiting.\n");
     exit(1);
#    endif /*ERROR_EXIT*/
     errors++;
     n_nuc = 1;
   }

   /* more nuclei than cells???  */
   if (n_nuc > bp->ncsb){
     fprintf(stderr,"WARNING: init_nuc_thresh: too many nuclei for %d! Resetting ...\n",inuc_law);
     n_nuc = bp->ncsb;
   }

   /* create the nuclei */
   fprintf(stderr, "init_nuc_thresh: subblock %i -- Number of nuclei will be: %i\n",sbnum,n_nuc);

   /* create pre-fixed nuclei threshold */
   for (i=0;i<n_fixed_nuc;i++) {
     index = (int) (cp->nsite[i][0]+cp->nsite[i][1]*nx+cp->nsite[i][2]*nxny);
     fprintf(stderr,"sb_nuc: adding fixed nuc at %i : threshold %.5g\n",index,cp->nsite[i][6]);
     if (index>bp->ncsb) {
       fprintf(stderr,"ERROR: fixed-nuc outside of subblock,%i\n",index);
       fprintf(stderr,"     : setting to zero.\n");
       index=0;
     }
     *(thr + index) =  cp->nsite[i][6];

/*THUINET 05/05*/
     *(nat_site + index) = cp->nsite[i][7];
/*END THUINET 05/05*/
   }

   /* create random nuclei threshold */

   int1 = n_fixed_nuc;
   int2 = n_fixed_nuc+n_nuc_poly[0];

   for (inuc_law = 0; inuc_law<bp->ctrl->NUM_NUC_LAW; inuc_law++) {

     for (i=int1;i<int2;i++) {

     /* find a new site that is not taken*/
       do{
         int tries=0;
         newnuc = (int)FLOOR(drand48()*bp->ncsb);
         if (tries++ > bp->ncsb ){
           fprintf(stderr,"ERROR: init_nuc_thresh: Double Aaargh!\nYou have to debug.\n");
           errors++;
           exit(1);
         }
       }while (*(thr + newnuc) > NUC_THRESH_NONE);

       *(nat_site+newnuc) = inuc_law;

       /****************************************************************************/
       /* find a gaussian devated, or other function, that is within #defined range*/
       /* paranoia check that it is greater than zero also!                        */
       /* but G_NUC_MIN_UND ought to be >= 0 anyways                               */
       /****************************************************************************/

       do{
         int tries = 0;
         /* random function set up on readmat.c as an option */
         /* using a pointer to one of the available random functions */
         threshold = (*(bp->nprops.rand_function))(bp->nprops.NucParams_poly[inuc_law]);
         if (tries++ > bp->ncsb ){
           fprintf(stderr,"ERROR: init_nuc_thresh: Could not find a nucleation site after %i tries.\n",tries);
           fprintf(stderr,"ERROR: init_nuc_thresh: Triple Aaargh!\nYou have to debug.\n");
           errors++;
           exit(1);
         }
       }while ( ( ABS(threshold - bp->nprops.NucParams_poly[inuc_law][0]) > G_NUC_SIG_MULT * bp->nprops.NucParams_poly[inuc_law][1]) || (threshold < G_NUC_MIN_UND) || (threshold < 0));

       n_created++;
       *(thr+newnuc) = threshold;
       #ifdef LIST_ALL_NUC
        getxyz (newnuc, bp->nc, Cell);
        fprintf (listfile, "%i,%i,%i,%.5g\n", Cell[0],Cell[1],Cell[2], threshold);
       #endif /*LIST_ALL_NUC*/
     }

     int1 = int2;
     int2+=n_nuc_poly[inuc_law+1];

   }/*end of loop on inuc_law*/

   /* nucleate at mould bdy */
   if(bp->ctrl->mould_nuc){
      errors+= init_nuc_mould(bp,sbnum,listfile);
   }
#  ifdef HISTO_NUC
     errors += nuchisto(listfile,bp,sp->c_nuc_thresh) ;
#  endif /*HISTO_NUC*/
      fclose(listfile);
      return(errors);

}/* end of init_nuc_threshold */


/******************************************************************/
/* nuchisto -- set up parameters and then call umat_histo */
/* to create a histogram of the nucleus threshold (input) */
/******************************************************************/
int nuchisto (FILE * listfile, BB_struct * bp, CA_FLOAT * thr)
{

  int errors = 0;
  int i;
  Histo_struct histoparams;

  histoparams.binsize = G_BINSIZE;
  histoparams.minbin = G_MINBIN;
  histoparams.nbins = G_NBINS;
  histoparams.ndata = bp->ncsb;
  for (i = 0; i < 4; i++)
    histoparams.stat[i] = 0;

  fprintf (listfile, "UndThr,,,,");
  for (i = 0; i <= G_NBINS; i++) {
    fprintf (listfile, ",%.5g", (G_MINBIN + (CA_FLOAT) (i) * G_BINSIZE));
  }
  fprintf (listfile, "\n");

  errors += umat_histo (listfile, &histoparams, thr, FIRST_CALL);

  return (errors);
}

/******************************************************************/
/******************************************************************/
/* block_cell_nuc: Compare with threshold to see if cel nucleates */
/******************************************************************/
/******************************************************************/
int block_cell_nuc (BB_struct * bp, CA_FLOAT Tunder, CA_FLOAT threshold)
{
/* BLOCK_NUC method */
  int nuc_flag = 0;
  static int nucmsg = 0, nonucmsg = 0;
  CA_FLOAT old_Tunder = 0;      /*dummy for THRESH_MESS macro */

#undef THIS_FUNC
#define THISFUNC "block_cell_nuc"
  if (Tunder > threshold && threshold >= 0)
    nuc_flag = 1;
  THRESH_MESS return (nuc_flag);

#undef THIS_FUNC
}                               /* end of block_cell_nuc */

/****************************************************************/
/****************************************************************/
/* cell_nuc: Calculate if single cell nucleates...              */
/****************************************************************/
/****************************************************************/
int cell_nuc (BB_struct * bp, CA_FLOAT Tunder, CA_FLOAT dummy)
{
  CA_FLOAT Tnuc, threshold, perturb, t_s, dt, time;
  static int nucmsg = 0;
  double tmp1;                  /* tmp double var. */
  double integral;
  int i, j;
  Nuc_str *np;

/* set all pointers to point to structure values  */
  np = &(bp->nprops);

/* other stuff -- not using pointers  */
  tmp1 = 1.0 / (SQRT2 * np->NucParams[1]);
  /* the density of new nuclei - per mm^3 per s */
  if (np->perturb_on) {
    perturb = 1.0;
    time = bp->sim_time;
    for (i = 0; i < np->n_perturb; i++) {
      /* check if active */
      t_s = np->perturb[i].v_f[1];
      dt = np->perturb[i].v_f[2];
      if ((time >= t_s) && (time < (dt + t_s))) {
        perturb += np->perturb[i].v_f[0];
      }
    }
    integral = 0.5 * np->gd_max * perturb * (erf ((double) (Tunder - np->NucParams[0]) * tmp1) +
                                             erf ((double) np->NucParams[0] * tmp1));
  } else {
    integral = 0.5 * np->gd_max * (erf ((double) (Tunder - np->NucParams[0]) * tmp1) + erf ((double) np->NucParams[0] * tmp1));
  }

  threshold = integral * bp->vol_c * bp->delt;
  if (((CA_FLOAT) drand48 ()) < threshold) {
    if (nucmsg < MAX_NUC_MSG) {
      fprintf (stderr, "Nucleating grain, threshold  = %.10g, Tund=  %.10g\n", threshold, Tunder);
      nucmsg++;
    }
    return (1);
  } else {
    return (0);
  }
}                               /* end of cell_nuc */

/****************************************************************/
/*                                                              */
/* del_cell_nuc: Calculate if single cell nucleates,            */
/* based on rappaz model using change in undercooling.          */
/*                                                              */
/****************************************************************/
int del_cell_nuc (BB_struct * bp, CA_FLOAT Tunder, CA_FLOAT old_Tunder)
{
#undef THISFUNC
#define THISFUNC "del_cell_nuc"
  CA_FLOAT Tnuc, threshold, t_s, dt, time;
  static int nucmsg = 0, nonucmsg = 0;
  int nuc_flag = 0;
  double tmp1;                  /* tmp double var. */
  double integral;
  int i, j;
  Nuc_str *np;

  np = &(bp->nprops);

  /*Precalculate preexponential */
  tmp1 = 1.0 / (SQRT2 * np->NucParams[1]);
  /* the density of new nuclei - per mm^3 per s */
  integral = 0.5 * np->gd_max * (erf ((double) (Tunder - np->NucParams[0]) * tmp1) -
                                 erf ((double) (old_Tunder - np->NucParams[0]) * tmp1));
  threshold = integral * bp->vol_c * bp->delt;
  if (((CA_FLOAT) drand48 ()) < threshold)
    nuc_flag = 1;

  /* print the threshold output */

  THRESH_MESS return (nuc_flag);

#undef THISFUNC
}                               /* end of del_cell_nuc */

/*******************************************/
/* as above but no time dependancy at all. */
/*******************************************/
int del_cell_nuc_norate (BB_struct * bp, CA_FLOAT Tunder, CA_FLOAT old_Tunder)
{
#undef THISFUNC
#define THISFUNC "del_cell_nuc_norate"
  CA_FLOAT Tnuc, threshold, t_s, dt, time;
  static int nucmsg = 0, nonucmsg = 0;
  int nuc_flag = 0;
  double tmp1;                  /* tmp double var. */
  double integral;
  int i, j;
  Nuc_str *np;

  np = &(bp->nprops);

  /*Precalculate stdevdev. factor */
  tmp1 = 1.0 / (SQRT2 * np->NucParams[1]);
   /*************************************************************/
  /* The change of the erf function, from old_Tunder to Tunder */
  /* which is d(N)/d(Tunder) * delta(Tunder) already!          */
   /*************************************************************/
  integral = 0.5 * np->gd_max * (erf ((double) (Tunder - np->NucParams[0]) * tmp1) -
                                 erf ((double) (old_Tunder - np->NucParams[0]) * tmp1));
  threshold = integral * bp->vol_c;     /* note: this line is different from above */
  if (((CA_FLOAT) drand48 ()) < threshold)
    nuc_flag = 1;
  /* print the threshold message */
  THRESH_MESS return (nuc_flag);

#undef THISFUNC
}                               /* end of del_cell_nuc_norate */

/*******************************************/
/* Calculate the probablity of nucleation  */
/* for a single cell based on the          */
/* distribution model.                     */
/*******************************************/
int dist_cell_nuc (BB_struct * bp, CA_FLOAT Tunder, CA_FLOAT dummy)
{

  CA_FLOAT nucrate;
  CA_FLOAT *nuctable;
  CA_FLOAT threshold;
  static int nucmsg = 0, nonucmsg = 0;
  int TundBin;
  Nuc_dist_str *ndp;

  ndp = &(bp->nprops.nucdist);
  nuctable = ndp->Nuctable;

  TundBin = (int) (Tunder * (ndp->Tund_factor));
  TundBin = MIN (TundBin, (ndp->n_Tund_bins - 1));
  nucrate = *(nuctable + TundBin);
  threshold = nucrate * bp->vol_c * bp->delt;

  if (((CA_FLOAT) drand48 ()) < threshold) {
    if (nucmsg < MAX_NUC_MSG) {
      fprintf (stderr, "dist_cell_nuc: Nucleating grain, threshold  = %.5g, T=  %f\n", threshold, Tunder);
      nucmsg++;
    }
    return (1);
  } else {
#ifdef VERBOSE_CELLNUC
    if (nonucmsg < MAX_NUC_MSG) {
      fprintf (stderr, "dist_cell_nuc: NOT Nucleating grain, threshold  = %.5g, T=  %f\n", threshold, Tunder);
      nonucmsg++;
    }
#endif /*VERBOSE_CELLNUC */
    return (0);
  }

}

/****************************************************************/
/****************************************************************/
/* sb_nuc_area:                                                 */
/* Place nuclei over an area at a given density                 */
/****************************************************************/
/****************************************************************/
int sb_nuc_area (BB_struct * bp, int sbnum, NucArea_struct * nap)
{
  int i, index;                 /* tmp counters */
  int nx, ny, nz, dx, dy, dz;   /* cell #'s and ofsets */
  int Xindex, Yindex, Zindex;
  int nnuclei;                  /* number of new nuclei formed... */
  CA_FLOAT tmp, vol;            /* tmp CA_FLOAT var. */
  CA_FLOAT oct_fact = bp->oct_factor;   /* tmp CA_FLOAT var. */
  double dtmp;                  /* tmp CA_FLOAT var. */
  SB_struct *sp;
  Ind_grain *g;
  FILE * listfile;
  int Cell[3];

  /* define external math routines */
#ifdef P_JUNK
  extern double drand48 ();
#endif

/************************************************/
/* Calculate the number of nuclei formed...     */
/************************************************/
  nx = nap->uright[0] - nap->lleft[0];
  ny = nap->uright[1] - nap->lleft[1];
  nz = nap->uright[2] - nap->lleft[2];
  vol = (nx + 1) * (ny + 1) * (nz + 1) * bp->vol_c;
  nnuclei = nap->dens * vol + 0.5;
/********************************************************/
/* Add the nuclei into cells randomly                   */
/********************************************************/
  fprintf (stderr, "SB_nuc_area [%d] adding %d new nuclei.\n", sbnum, nnuclei);
  listfile=fopen("G_L_AREA.csv","w");
  for (i = 0; i < nnuclei; i++) {
    dx = (int) (drand48 () * (nx + 1));
    dx = MIN ((dx), (nx));
    dy = (int) (drand48 () * (ny + 1));
    dy = MIN ((dy), (ny));
    dz = (int) (drand48 () * (nz + 1));
    dz = MIN ((dz), (nz));
    Xindex = nap->lleft[0] + dx;
    Yindex = (nap->lleft[1] + dy);
    Zindex = (nap->lleft[2] + dz);
    index = Xindex + Yindex * bp->nc[0] + Zindex * bp->nc[0] * bp->nc[1];
    if (index > bp->ncsb) {
      fprintf (stderr, "ERROR (sb_nuc_area): index out of range [%d]\n", index);
    } else {

      if (bp->sb[sbnum]->c_fs[index] == LIQUID) {
        if (bp->nprops.ngr == bp->nprops.gd_max_total) {
          fprintf (stderr, "ERROR: sb_nuc - Max # grains set by user [%d] exceeded. Increase option MaxTotGrains.\n",
                   bp->nprops.gd_max_total);
        } else {
          /* changed to set the threshold, then let the ordinary */
          /* nucleation routine handle the grain */
          bp->sb[sbnum]->c_nuc_thresh[index] = 0.0001;
#ifdef LIST_ALL_NUC
#ifdef LIST_ALL_NUC
     fprintf (listfile, "%i,%i,%i,%.5g\n", Xindex,Yindex,Zindex, .0001);
#endif /*LIST_ALL_NUC */
#endif
#ifdef JUNK
          bp->sb[sbnum]->c_fs[index] = EMBRYO;
          bp->nprops.ngr += 1;
          bp->sb[sbnum]->ngr += 1;
          bp->sb[sbnum]->gr[index] = bp->nprops.ngr;
          /* init the grain structure for that grain... */
          init_new_grain (bp, bp->nprops.ngr, sbnum, Xindex, Yindex, Zindex, 1);
#endif     /*JUNK*/
        }
      }
    }
  }
  fclose(listfile);

  return (0);
}                               /* end of sb_nuc_area subroutine */

/*#undef VERBOSE_NUC_MSG*/

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file*/
char const *rcs_id_sb_nuc_c ()
{
  static char const rcsid[] = "$Id: sb_nuc.c 1398 2008-11-13 11:37:26Z  $";

  return (rcsid);
}

/* end of rcs_id_sb_nuc_c subroutine */
/*
*/
