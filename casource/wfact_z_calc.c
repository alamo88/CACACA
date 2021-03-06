
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
/* wfact_z_calc.c:                                              */
/* Subroutine to calculation the weighting factors for cells    */
/* position against the nodes position                          */
/****************************************************************/
/* Written by X. Xu, P.D. Lee & R.C. Atwood, Imperial College   */
/* Feb. 18, 2000                                                */
/****************************************************************/
/*      MODIFIED by:                                            */
/****************************************************************/
/****** To Do List **********************************************/
/*General:                                                      */
/* 1)                                                           */
/****************************************************************/
#include <stdio.h>
#include <math.h>
/* include header files requred by subroutines */
#include "machine.h"
#include "blocks.h"
#include "fidap.h"              /* included for def. of FGrid_struct */
#include "nearnode.h"
/*subroutines later in this file*/
int search_z (FGrid_str * fg, CA_FLOAT z);
int compare_z (FGrid_str * fg, int pn, CA_FLOAT z);

/****************************************************************/
/****************************************************************/
/* wfact_z_calc.c:                                             */
/* Subroutine to calculate weighting factor of cells in height  */
/* direction                                                    */
/****************************************************************/
/* Input Variables:                                             */
/*   *fg:		ptr to the FGrid_str structure that     */
/*       		holds all FIDAP geometry and T's data.  */
/*   *bp:		hold location of cellsin the CA model   */
/*   time:		time in the CA model             [s]    */
/*      		                                        */
/* Output Variables:    weight factor in height direction.      */
/*      		                                        */
/* Returned Value:      NONE                                    */
/****************************************************************/
void wfact_z_calc (FGrid_str * fg, NODENB_str * node_ptr, BB_struct * bp, int sbnum)
{
  CA_FLOAT zc, zm;
  int i, ii, ic, imax, pnd, nnz;
  CA_FLOAT sub_off, trans_off, cell_offset, size_cell, *fg_z_pt;
  CA_FLOAT time;

  time = bp->sim_time;
  /* set pointers and variables for local usage */
  fg_z_pt = fg->z;
  nnz = fg->nz;
  trans_off = fg->z_off;        /* offset for start CA calculation within melt pool */
  sub_off = bp->sb[sbnum]->orig_sb[0];  /*     lower left corner of the subblock     */
  size_cell = bp->size_c[0];    /* cell size in x direction, it equals to that in y & Z dir. */
  cell_offset = sub_off - 0.5 * size_cell;      /* center of the cell in front of it. */

  /* calculate z arrays of cells in Z direction */
  zc = cell_offset;             /* initial value */
  imax = bp->nc[0];

  for (i = 0; i < imax; i++) {
    zc += size_cell;

    /* calculate Z position of cell in a CFD domain */

    if (fg->state == TRANSIENT) {
      zm = fg->h_ingot - trans_off - zc;
    } else if (fg->state == STEADY) {
      zm = -zc + fg->v * time;
    }

         /*********************************************************************/
    /*  check if the CA cell is out of the CFD domain, if it is over the */
    /*  top of  CFD domain, set node No.= -1, therefore use  it in       */
    /*  temperature interpolation subroutine for Tcell = Tmax, otherwise */
    /*  if below the bottom of CFD domain , set node No.the Maxium and   */
    /*  T = Tmin                                                         */
         /*********************************************************************/

    if (zm <= 0.0) {
      node_ptr->nd[i] = -1;
    } else if (zm > fg->h_ingot - trans_off) {
      node_ptr->nd[i] = fg->nz - 1;
    } else {

          /******************************************************************/
      /* Make a bisection search for the nearest node in the first step, */
      /*  compare zm of cell with the nearest node in other steps.      */
          /******************************************************************/

#ifdef JUNK
      if (bp->step < 1) {
#endif
        ic = search_z (fg, zm);
#ifdef JUNK
      } else {
        pnd = node_ptr->nd[i];
        ic = compare_z (fg, pnd, zm);
      }
#endif
      node_ptr->nd[i] = ic;     /* nearest node down the cell.  */
           /****************************************************/
      /* factor that the cell is away from the down node, */
      /* when it lies on the node, wd[i] = 1.             */
           /****************************************************/
      node_ptr->wd[i] = (fg_z_pt[ic + 1] - zm) / (fg_z_pt[ic + 1] - fg_z_pt[ic]);
      node_ptr->wu[i] = 1 - node_ptr->wd[i];    /* factor from upper nodes. */
    }
  }
/* Debug only */
  /*       if ((node_ptr->wd[i] > 1) || (node_ptr->wd[i] < 0 )) {
     fprintf(stderr,"ERROR wd, i [%f %d]", node_ptr->wd[i], i);
     }
   */
}                               /*  end of wfact_z_calc    */

/* subroutine for search the node before the CA cell  */

int search_z (FGrid_str * fg, CA_FLOAT z)
{
  int i, im, iu, il, JP;
  CA_FLOAT *fg_z_pt;

  fg_z_pt = fg->z;

  iu = fg->nz - 1;
  il = 0;
  while (iu - il > 1) {
    im = (iu + il) / 2;
    if ((z > fg_z_pt[il]) && (z > fg_z_pt[im])) {
      il = im;
    } else {
      iu = im;
    }
  }
  JP = il;
  return (JP);
}

/* Subroutine for compare the coordicate of the cell with the node coor. in current step */
int compare_z (FGrid_str * fg, int pn, CA_FLOAT z)
{
  int i, imax, JP;
  CA_FLOAT *fg_z_pt;

  fg_z_pt = fg->z;
  JP = pn;
  imax = 2;

  for (i = imax; i >= 1; i--) {
    if (z > fg_z_pt[pn + i]) {
      JP = JP + i;
      break;
    }
  }
  return (JP);
}

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file*/
char const *rcs_id_wfact_z_calc_c ()
{
  static char const rcsid[] = "$Id: wfact_z_calc.c 1341 2008-07-23 15:23:30Z  $";

  return (rcsid);
}

/* end of rcs_id_fidap_interp_calc_c subroutine */

/*RCS Id:$Id: wfact_z_calc.c 1341 2008-07-23 15:23:30Z  $*/
/*
*/
