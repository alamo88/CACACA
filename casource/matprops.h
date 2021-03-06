
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
/* matprops.h                                                   */
/* Header file defining material property related structures    */
/****************************************************************/
/****************************************************************/
/* Written by Peter D. Lee & Robert C. Atwood, Imperial College */
/* Wed Jul  1 18:38:31 bst 1998                                 */
/****************************************************************/
/*RCS Id:$Id: matprops.h 1341 2008-07-23 15:23:30Z  $*/

#ifndef MATPROPS_H
#define MATPROPS_H

#include "solprops.h"

/****************************************************************/
/* The material properties structure, which holds     */
/* values which are constant for the whole big block  */
/****************************************************************/

#define MATPROPREV "matprop.h $Revision: 1341 $"
typedef struct matprop {
                        /*************MATERIAL PROPS*************/
	
        CA_FLOAT tp;               /* temperature of pure component in binary system */
        CA_FLOAT Tliq;             /* liquidus temperature [K]             */
	CA_FLOAT Tsol;             /* solidus temperature [K]              */
        CA_FLOAT Tstart_Offset;    /* the offset value added to the liquidus temperature */
	CA_FLOAT Tstart;           /* Tliq plus offset used for opening sb's*/
	CA_FLOAT latentH;          /* latent heat [J/mm^3]                 */
	CA_FLOAT rho;              /* density [g/mm^3]                     */
	CA_FLOAT cp;               /* Cp [J/g/K]                           */
	CA_FLOAT rhocp;            /* rho * Cp [J/mm^3/K]                  */
	CA_FLOAT CA_Q;             /* heat flux */
	CA_FLOAT QSV;              /*  Q*surf area/vol          */
	CA_FLOAT surf_tens;        /* Surface tension [mK] use for Z-recry */
        CA_FLOAT gibbs_thomson;    /* coefficient de Gibbs-Thomson for the calculation of the curvature undercooling */
	CA_FLOAT gg_const;		/* grain growht const M/s/K            */
	CA_FLOAT gg_cub;        /*coefficient of t^3 in grain growth eqn*/
        CA_FLOAT dt_melt;       /*amount of superheating for remelting*/

        int stoechio; /*indicate if the secondaryr phase is stoechiometrique or not*/


	/* Moved the properties to individual structure for each component */
	/* see solprops.h */
	/* included as nested structure (not pointer) to simplify the writing */
	/* and reading of blk restart files */
	Solute_props gasprops;
	int ele_1;  /* number of solutes equals number of total components minus 1 */  
	            /* not counting gas */
	Solute_props alloyprops[NSOLMAX];
        
        CA_FLOAT Tliq_poly[NPHAMAX]; /* One value per phase */
        CA_FLOAT tp_poly[NPHAMAX];   /* Effective pure melting point One value per phase */

	CA_FLOAT das_factor; /* geometric factor for das ?? */
	CA_FLOAT das;  /* calculated DAS for limiting pressure calc. */
                	/* constants to define simplified binary phase diagram */
} Mat_str;

#endif /* MATPROPS_H */
/*
Trimmed the in-file log. Use cvs log facility to see the entire log.
*/
