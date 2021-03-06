
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include "machine.h"
#include "blocks.h"
#include "pore.h"
#include "props.h"
#include "rand_step.h"
#include "gaussdev.h"
#include "umat_histo.h"
extern CA_FLOAT global_pressure;
extern int getxyz (int cellnum, int *nc, int *Cell);

/* also find the avg. frac. solid */
	int cell_find_nmols (BB_struct * bp, int sbnum, PORE_str * c_p, CA_FLOAT * fs, CA_FLOAT * sol, CA_FLOAT * alloy, int Cell[3],
			     int cellnum)
	{
	  CA_FLOAT km;
	  int errors = 0;

	  CA_FLOAT cell_vol, cell_temp, cell_tempK, cell_fs, cell_fl, cell_sol, cell_phi, cell_si;
	  CA_FLOAT cell_satSI, p_pres, p_sat;
	  CA_FLOAT extra_h, extra_nmols;


  /* km is short for 1-partcoef */
  km = bp->mprops.gasprops.km;



  /* figure out the temperature */
  cell_temp = bp->sb[sbnum]->c_temp[cellnum];

  /* collect the attributes of the cell */
  cell_vol = bp->vol_c;
  cell_tempK = cell_temp + STD_TEMP;    /* in Kelvin */
  cell_fs = (*(fs + cellnum));  /* the fraction solid of the cell */
  cell_fl = 1 - (*(fs + cellnum));      /* ditto fraction liqid */
  cell_sol = (*(sol + cellnum));        /* and the sol (hydrogen) content (cl) */

  /*lever rule for how much H in mushy cell */
  cell_phi = 1 - (km * cell_fs);

  /* figure out the alloy solute concentration (si or mg or cu etc)   */
  /* procedurize? */
  cell_si = 0;
  if (bp->ctrl->scheil == TRUE) {       /*schiel mode -- calculate alloy */
    cell_si = find_sch_conc (cell_tempK, cell_fl);
  } else {                      /*not scheil mode -- get alloy from array */
    if ((bp->ctrl->diffuse_alloy == TRUE) && (bp->ctrl->diffuse_alloy_multi == FALSE)) {
      cell_si = (*(alloy + cellnum));
    }
  }                             /* end scheil/notschiel */

  /* call the routine from props.c to find the saturation limit */
  if ((bp->ctrl->diffuse_alloy == TRUE) && (bp->ctrl->diffuse_alloy_multi == FALSE)) {
    cell_satSI = find_sat (&(bp->mprops), cell_tempK, cell_si, cell_fs);
  }

  if (bp->ctrl->diffuse_alloy_poly == TRUE) {
    cell_satSI = find_sat_poly (bp, sbnum, cell_tempK);
  }


  /* calculate the excess hydrogen */
  p_pres = c_p->Pressure;
  p_sat = SQRT (p_pres / (global_pressure * P_AP_PA)) * cell_satSI;
  extra_h = cell_phi * (cell_sol - p_sat);
  extra_nmols = extra_h * (cell_vol);   /* mols of h atoms */
  c_p->extra_nmols += extra_nmols;
  c_p->sat = p_sat;

  /* and hence calculate the supersaturation ratio */
  c_p->supersat = (cell_sol / cell_satSI);

  /* add in the fraction solid to check if cell is frozen */
  c_p->FracSol += cell_fs;

  return (errors);
}

int gather_hydrogen (p_c_list * list)
{
/* function stub */
  return (100);
}

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file */
char const *rcs_id_find_nmols_c ()
{
  static char const rcsid[] = "$Id: find_nmols.c 1373 2008-08-27 20:51:52Z  $";

  return (rcsid);
}

/* end of rcs_id_subroutine */
