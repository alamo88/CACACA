
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

/*RCS Id:$Id: sb_diffuse_gas_poly.c 1356 2008-08-18 13:41:15Z  $*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "blocks.h"
#include "machine.h"
#include "umat_matrix.h"
#include "sb_diffuse.h"
#include "props.h"
#include "mould_sources.h"

#define MAX_DIFF_WARN 10

#ifdef CHECK_GAS
extern CA_FLOAT checkgas (BB_struct * bp, int callflag);
#endif

int sb_diffuse_gas_poly (BB_struct * bp, int sbnum)
{
  int dumb = 0;
  int index_ca;
  int printflag, errflg = 0, fileflag = 0, errors = 0;
  int nx, ny, nz, tsteps;
  int *oni, *onip, *onend;
  SB_struct *sp;
  Ctrl_str *cp;
  int t, i, j, k, l;
  static int warn_msg = 0;
  CA_FLOAT new_sol, OldS, NewS, partcoef,r_eff, km;
  CA_FLOAT *c_temp_p;
  CA_FLOAT **sol_gas_values;
  CA_FLOAT *oldfs, *oldfs_start, *newfs, *newfs_start;
  CA_FLOAT old_alpha, new_alpha, alpha_ratio, conc;     /*readability variables */
  CA_FLOAT *np, *op, flux, nbconc, *nsol, *osol, dxsq, dt, nbsum;
  CA_FLOAT rmax, rmin, tmax, tmin;
  CA_FLOAT r, rs, rl, ds, rsrl, dl, cellds, celldl, dtx, dmax, dmin, d; /*solid, liquid */
  int iphs, iphs_tot;
  CA_FLOAT *newfs_poly[NPHAMAX], *oldfs_poly[NPHAMAX];
  CA_FLOAT nbfs[NPHAMAX],fs_av[NPHAMAX];

#ifdef CHECK_GAS
  static CA_FLOAT diffgas0 = 0, diffgas1 = 0, change = 0;

  diffgas0 = checkgas (bp, 3);
  change = diffgas0 - diffgas1;
  if (ABS (change) > 1e-5) {
    fprintf (stderr, "WARNING:%s gas change limit exceeded! %.5g\n",__func__, change);
  }
#endif

  cp = bp->ctrl;
  iphs_tot = cp->NUM_PHS;



/* set up local neighbourhood */
/* use 6cell only for now     */
  rmin = dmin = 1;
  rmax = dmax = 0;
  tmax = 0;
  tmin = 10000;
  oni = bp->nbhd.onq;           /*padded */
  onip = oni;
  onend = oni + 6;
/* set up local values and pointers */
  printflag = ((bp->step % bp->ctrl->scr_dmp_freq));

  sp = bp->sb[sbnum];
  bp->cubeptr.curr = sbnum;
  nx = bp->nc[0];
  ny = bp->nc[1];
  nz = bp->nc[2];
  op = osol = bp->ftmp_two;     /* gas solute */
  np = nsol = sp->c_sol;        /* gas solute */
  c_temp_p = sp->c_temp;        /* temperature of cell */
  sol_gas_values = bp->c_sol_values->block_array;

  if (bp->ctrl->scheil == TRUE) {
    newfs = newfs_start = bp->ftmp_three;
    oldfs = oldfs_start = sp->sch_fs;
  } else {
    newfs = newfs_start = bp->ftmp_one;
    oldfs = oldfs_start = sp->c_fs;
  }                             /*end of schiel test */


  for (iphs = 0; iphs < iphs_tot; iphs++) {
    newfs_poly[iphs]= bp->ftmp_one_poly[iphs];
    newfs_poly[iphs] += bp->cubeptr.flist[0][START];
    oldfs_poly[iphs] = sp->c_fs_poly[iphs];
  }

  tsteps = bp->ctrl->diffuse_step;
  dxsq = bp->size_c[0] * bp->size_c[0];
  r = 0;
  /*delta t over delta x squared */
  dtx = (bp->delt / (dxsq * tsteps));

/* get the padded frac solid array to check */

/* make a copy of the solute concentration array */

  bp->cubeptr.ivalue = bp->Cbdy_gas;
  errflg += fcopy_matrix (PAD, osol, nsol, bp, sol_gas_values, sbnum);  /* (flag, to, from, bp) */
  bp->cubeptr.ivalue = 0.0;

  /* add the source at the mould surface if selected */
  if (bp->mprops.gasprops.mould_src != 0) {
    sb_mould_src (bp, &(bp->mprops.gasprops), sbnum, osol);
  }
  /* rewind array pointers */
  np = nsol;
  op = osol + bp->cubeptr.flist[0][START];      /*rewind */
  newfs = newfs_start + bp->cubeptr.flist[0][START];
  oldfs = oldfs_start;
  
  /* get the initial temperature for comparison */
  tmin = *c_temp_p;

  /************************************************/
  /* now calculate the finite difference          */
  /************************************************/
  /* DIFFUSION LOOP                               */
  /************************************************/
  /* Run through all cells updating as needed.    */
  /************************************************/

  for (k = 0, index_ca = 0; (k < nz && index_ca < bp->tnc[0] * bp->tnc[1] * bp->tnc[2]); k++) { /* loop cells in z direction */
    for (j = 0; j < ny; j++) {  /* loop cells in y direction */
      for (i = 0; i < nx; i++) {        /* loop cells in x direction */

        /* update concentration for diffusion, using part. coeff */

        /* skip cells that are not in the casting */
        if (*oldfs != NOT_CASTING) {

          old_alpha = 1.0;
          new_alpha = 1.0;

          for (iphs = 0; iphs < iphs_tot; iphs++) {
            old_alpha -= (1-bp->mprops.gasprops.part_coef[iphs])**oldfs_poly[iphs];
            new_alpha -= (1-bp->mprops.gasprops.part_coef[iphs])**newfs_poly[iphs];
          }
          alpha_ratio = (old_alpha / new_alpha);

          nbsum = 0;
          conc = *op;
#ifdef CELL_DIFF_ARR
          celldl = get_dl (*c_temp_p, &(bp->mprops.gasprops));
          cellds = get_ds (*c_temp_p, &(bp->mprops.gasprops));
#else
          celldl = get_dl (*c_temp_p);
          cellds = get_ds (*c_temp_p);
#endif
          rs = cellds * dtx;
          rl = celldl * dtx;

          /* check for error/warning conditions */
          if ((rs > COURANT_LIMIT || rl > COURANT_LIMIT)) {
            if (warn_msg < MAX_DIFF_WARN) {
              fprintf (stderr, "SB_DIFFUSE_GAS: WARNING: courant values too high!, %1.2e,%1.2e\n", rs, rl);
            }
            if (warn_msg > WARN_EXIT_LIMIT) {
              fprintf (stderr, "ERROR: sb_diffuse_gas: Warning limit exceeded. Exiting. %i\n", warn_msg);
              exit (warn_msg);
            }

            warn_msg++;
          }
          /* end of warnign check */

          for (onip = oni; onip < onend; onip++) {      /* neighbour sum loop */

            for (iphs = 0; iphs < iphs_tot; iphs++) {
              nbfs[iphs] = *(newfs_poly[iphs] + *onip);

              /* skip nb cells that are not in the casting */
              if (nbfs[iphs] == NOT_CASTING)
                continue;

              /* averaged frac solid */
              fs_av[iphs] = 0.5 * (*newfs_poly[iphs] + nbfs[iphs]);
            }/*End loop on the solid phases */

            d = getav_d (celldl, cellds, fs_av[0]);
            /*diffusion in the second stoechiometric phase is not taken into account*/
            
            r = dtx * d;
            nbconc = *(op + *onip);
            nbsum += r * (nbconc - conc);
            /* get values to print for monitoring */
            if (printflag == 0) {
              if (d > dmax)
                dmax = d;
              if (d < dmin)
                dmin = d;
              if (r > rmax)
                rmax = r;
              if (r < rmin)
                rmin = r;
              if (*c_temp_p > tmax)
                tmax = *c_temp_p;
              if (*c_temp_p < tmin)
                tmin = *c_temp_p;
            }
          }                     /* end of neighbour sum loop */
          *np = alpha_ratio * *op + nbsum / new_alpha;
          #ifdef TRAP_BIG_GAS
          { 
              int dumb;
              if (*np > TRAP_BIG_GAS || -(*np) > TRAP_BIG_GAS){
                dumb = 1;
              }
          }
          #endif
        }
        /* end of NOT_CASTING test */
        c_temp_p++;
        np++;
        index_ca++;
        oldfs++;
        newfs++;


        for (iphs = 0; iphs < iphs_tot; iphs++) {
          oldfs_poly[iphs]++;
          newfs_poly[iphs]++;
        }

        op++;
      }                         /*x */
      op += 2;
      newfs+=2;

      for (iphs = 0; iphs < iphs_tot; iphs++) {
        newfs_poly[iphs] += 2;
      }
    }                           /*y */
    op += 2 * (nx + 2);
    newfs +=2 *( nx + 2 );

    for (iphs = 0; iphs < iphs_tot; iphs++) {
      newfs_poly[iphs] += 2 * (nx + 2);
    }
  }                             /*z */

  if (printflag == 0) {
    fprintf (stderr, "sb_diffuse_gas: dmax %.5g  dmin %.5g %i\n", dmax, dmin, bp->step);
    fprintf (stderr, "sb_diffuse_gas: rmax %.5g  rmin %.5g\n", rmax, rmin);
    fprintf (stderr, "sb_diffuse_gas: tmax %.5g  tmin %.5g\n", tmax, tmin);
  }
#ifdef CHECK_GAS
  diffgas1 = checkgas (bp, 4);
  change = diffgas1 - diffgas0;
  if (ABS (change) > 1e-5) {
    fprintf (stderr, "WARNING:%s gas change limit exceeded! %.5g\n",__func__, change);
  }
#endif

  return (errflg);
}                               /* end of sb_diffuse */

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file*/
char const *rcs_id_sb_diffuse_gas_poly_c ()
{
  static char const rcsid[] = "$Id: sb_diffuse_gas_poly.c 1356 2008-08-18 13:41:15Z  $";

  return (rcsid);
}

/* end of rcs_id_sb_diffuse_gas_c subroutine */

/*
*/
