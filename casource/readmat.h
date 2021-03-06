
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
/* readmat.h                                                    */
/* Header file defining default values via cpp operatives       */
/****************************************************************/
/****************************************************************/
/* Written by Peter D. Lee & Robert C. Atwood, Imperial College */
/* Aug 16, 1998                                                 */
/*RCS ID$Id: readmat.h 1341 2008-07-23 15:23:30Z  $*/
/****************************************************************/
#ifndef READMAT_H
#define READMAT_H

/****************************************************************/
/* Define all of the default values below to be used            */
/* if the user does not specify them.                           */
/****************************************************************/
#define D_TSTART_OFFSET 5         /* the offset value added by default to the liquidus temp */
#define D_TLIQ 1180.0             /* liquidus temperature [C]     */
#define D_TSOL 1120.0             /* solidus temperature [C]     */
#define D_TEUT 1000.0           /* eutectic temperature [C]      */ 
#define D_LATENTH 1.44          /* latent heat [J/mm^3]         */
#define D_RHO 7700.0              /* density [g/mm^3]             */
#define D_CP 0.83               /* Cp [J/g/K]                   */
#define D_Q 0.05                /* Heat flux [W/mm^2]           */
#define D_SURF_TENS 0.9e-7      /* Surface tension [mK]         */
#define D_GG_CONST 1e-5       /* rain growht const M/s/K     */
#define D_GG_CUB 0.0
#define D_CINIT 1.0            /* initial concentration gas   */
#define D_DLIQ 1.0e-8           /*     */
#define D_DSOL 1.0e-9       /*      */
#define D_PART_COEF 0.1       /*      */
#define D_CINIT_ALLOY 1.0;
#define D_DLIQ_ALLOY 1.0e-9;
#define D_DSOL_ALLOY 1.0e-13;
#define D_PART_COEF_ALLOY 0.13;
#define D_M_ALLOY -7.1212;
#define D_GIBBS_THOMSON 1e-7   /*by Wei WANG 11-07-02*/
/* porpsity defautls*/
#define D_PTSIG 0.1       /*      */
#define D_PLIMRAD 2e-5 /* */
#define D_PTN  2      /*      */
#define D_PNPHAMAX 5.0e+9       /*      */
#define D_BINSIZE 0.01       /*      */

/****************************************************************/
/* Define all of the default values below to be used            */
/* if the user does not specify them.                           */
/****************************************************************/
#define D_NUC_MODEL 1           /* Use Rappaz model by default  */
#define D_GD_MAX 60.0           /* max. grain density per [mm^3]*/
#define D_GD_MAX_BEUT 60.0           /* max. grain density per [mm^3]*/
#define D_GD_MAX_TEUT 60.0           /* max. grain density per [mm^3]*/
#define D_TN 20.0               /* centre of gaussian [K]       */
#define D_TSIGMA 5.0            /* variation in gaussian [K]    */
                                /* Variables for HETEROGENIOUS  */
#define D_N_HET  20             /* number of het. nuclei        */

#endif /* READMAT_H */

/*
*/
