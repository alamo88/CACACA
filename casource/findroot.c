
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
#include <math.h>
#include "blocks.h"
#include "machine.h"

#define MAXROOT 0.1
#define MINROOT -0.1
#define MAX_ITERATIONS 100
#define CONVERGE_LIMIT 1e-10

/* find the radius using Newton's method , adapted from Numerical Recipes in C by Press and Flannery */

CA_FLOAT find_new_radius (BB_struct * bp, CA_FLOAT n, CA_FLOAT p, CA_FLOAT tempK, CA_FLOAT r)
{
  CA_FLOAT f, df, dx, sigma, rold;
  int i;

  sigma = bp->mprops.surf_tens;

  for (i = 0; i < MAX_ITERATIONS; i++) {
    f = FOURPI_BY_THREE * (p * r * r * r + 2 * sigma * r * r) - n * GAS_CONST_SI * tempK;
    df = FOURPI_BY_THREE * (3 * p * r * r + 4 * sigma * r);
    dx = f / df;
    rold = r;
    r -= dx;
#ifdef TEST_FINDROOT
    printf ("r: %.10g, r-rold %.10g\n", r, r - rold);
#endif

    if ((MAXROOT - r) * (r - MINROOT) < 0.0) {
#ifdef ERROR_EXIT
      fprintf (stderr, "ERROR: find_new_radius: exceeded bounds on radius.\n");
      exit (10);
#endif
      return (r);
    }
    if (ABS (r - rold) < CONVERGE_LIMIT) {
#ifdef TEST_FINDROOT
      printf ("Niter: %i\n", i);
#endif
      return (r);
    }
  }
#ifdef ERROR_EXIT
  fprintf (stderr, "ERROR: find_new_radius: exceeded maximum iterations.\n");
  exit (10);
#endif
  return (r);
}

#ifdef TEST_FINDROOT
void main ()
{
  BB_struct bb;
  CA_FLOAT p, tempK, r, n, rNew;

  bb.mprops.surf_tens = 0.856;
  p = 101300.0;
  r = 1e-6;
  tempK = 600;

  for (;;) {
    printf ("Input nmols:\n");
    scanf ("%f", &n);
    rNew = find_new_radius (&bb, n, p, tempK, r);
    printf ("Rnew = %.5g\n", rNew);
    r = rNew;
  }

}
#endif

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file */
char const *rcs_id_findroot_c ()
{
  static char const rcsid[] = "$Id: findroot.c 1339 2008-07-23 13:58:29Z  $";

  return (rcsid);
}

/* end of rcs_id_subroutine */
