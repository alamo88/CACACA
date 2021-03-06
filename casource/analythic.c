
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

/*RCS Id:$Id: analythic.c 1356 2008-08-18 13:41:15Z  $*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "blocks.h"
#include "matprops.h"
#include "multi_diff_props.h"
#include "machine.h"
#include "umat_matrix.h"
#include "sb_diffuse.h"

int analythic (BB_struct * bp, int k, int j, int i)
{
  int ele_num, ele_1;
  int nx, ny, nz, tsteps;
  SB_struct *sp;
  Ctrl_str *cp;
  MultiS_struct *ms;
  int t, l, m, n;
  CA_FLOAT r, sigma_1, sigma_2, Cons_1, Cons_2, Cons_3, Cons_4, tm;
  CA_FLOAT C1_bar, C2_bar, C1_inf, C2_inf;
  CA_FLOAT term1, term2, term3, term4, term5, term6, term7, term8;
  CA_FLOAT value_1, value_2, *CONC;
  CA_FLOAT D11, D22, D12, D21;

  CONC = bp->conc_anal;
  ms = &(bp->MultiSvals);
  cp = bp->ctrl;
  ele_num = cp->NUM_COMP;
  ele_1 = ele_num - 1;

  nx = bp->nc[0];
  ny = bp->nc[1];
  nz = bp->nc[2];

  if (ele_1 == 2) {

    C1_bar = (ms->Cinit_multi[0]);
    C2_bar = (ms->Cinit_multi[1]);

    C1_inf = 0.0;
    C2_inf = 0.0;

/**************calculation of sigma_1 and sigma_2*********/

    D11 = ms->Diff_matrix_liq[0][0];
    D22 = ms->Diff_matrix_liq[1][1];
    D12 = ms->Diff_matrix_liq[0][1];
    D21 = ms->Diff_matrix_liq[1][0];

    term1 = D11 + D22;
    term2 = POW ((D11 - D22), 2);
    term3 = 4 * D12 * D21;

    term4 = sqrt ((term2 + term3));

    sigma_1 = 0.5 * (term1 + term4);
    sigma_2 = 0.5 * (term1 - term4);

/**********************calculation of the 4 constants of the erf functions***/

    term5 = (D11 - sigma_2) * (C1_inf - C1_bar) + D12 * (C2_inf - C2_bar);
    term6 = D21 * (C1_inf - C1_bar) + (D22 - sigma_2) * (C2_inf - C2_bar);
    term7 = sigma_1 - sigma_2;

    Cons_1 = term5 / term7;
    Cons_2 = -term5 / term7;

    Cons_3 = term6 / term7;
    Cons_4 = -term6 / term7;

/***********************calculation of the concentrations************/

    r = POW (k, 2) + POW (j, 2) + POW (i, 2);
    r = sqrt (r);
    r = (1.0e-5) * r;

    tm = (bp->delt) * (bp->step);

    value_1 = r / (sqrt (4.0 * sigma_1 * tm));
    value_2 = r / (sqrt (4.0 * sigma_2 * tm));

    value_1 = erf (value_1);
    value_2 = erf (value_2);

    CONC[0] = C1_bar + (Cons_1 * value_1) + (Cons_2 * value_2);
    CONC[1] = C2_bar + (Cons_3 * value_1) + (Cons_4 * value_2);
  }
  if (ele_1 == 1) {
    C1_bar = (ms->Cinit_multi[0]) / 2.0;
    C1_inf = ms->Cinit_multi[0];
    D11 = ms->Diff_matrix_liq[0][0];
    r = POW (k, 2) + POW (j, 2) + POW (i, 2);
    r = sqrt (r);
    r = (1.0e-4) * r;
    tm = (bp->delt) * (bp->step);
    value_1 = r / (4.0 * D11 * tm);
    value_1 = erf (value_1);
    CONC[0] = C1_bar + (C1_inf - C1_bar) * value_1;
  }

  return (1);

}

/*****end of analythic subroutine****/

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file*/
char const *rcs_id_analythic_c ()
{
  static char const rcsid[] = "$Id: analythic.c 1356 2008-08-18 13:41:15Z  $";

  return (rcsid);
}

/* end of rcs_id_analythic_c subroutine */
