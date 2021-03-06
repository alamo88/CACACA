
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

#ifndef SUBBLOCK_H
#define SUBBLOCK_H
#define SUBBLOCKREV "subblock.h $Revision: 1373 $"

typedef struct subblock {
   int Npores;          /*Total possible pores in sb    */
   int code;            /* the code for which edge etc            */
   int done;            /* TRUE if finished substeps...           */
   int ncsolid;         /* number of solid cells in sb            */
   int ngr;             /* number of grains in sb                 */
   int nouts;           /* The flag for edge/cornr etc            */
   int nucnum;          /* number of grain boundary cells in sb   */
   int nmould;       /* number of "not casting" cells in sb    */
   int num;             /* subblock number                        */
   int open;            /* SB_NEW or SB_OPEN or SB_CLOSED   */
   int sb_npores;       /* number of active pores in sb */
   int orig_bb_idx[3];   /* big-block index of origin of the sb */
   CA_FLOAT orig_sb[3];    /* lower left front origin of sb in real space    */
   CA_FLOAT addsol;    /* amount of solute added in the step */
   CA_FLOAT t_addsol;  /* total amount of solute added since start */

   NODENB_str *nnd;     /* define for temperature interpolation. */
   NODENB_str *nnd_next;     /* define for temperature interpolation. */
   PORE_str *porelist;  /* list of cells with precalc. pore nuc.  */
   SB_nuc_str sbnuc;    /* defined in nucprops.h                  */
   S_struct Svals[2];      /* solute values in sb; see solprops.h    */
   T_struct Tvals;      /* temp and fl values in sb;see temprops.h*/
   SurCell surface;

   int *c_elm;          /* ptr to array of cell elm refs          */
   int *gr;             /* ptr to array of cell grain #           */
   int *index;          /* ptr to array of cell index:0=liq, 1=grain boundary, 2=inside grain*/ /*by Wei WANG on 11-07-02*/
/*   int *interface_index; REMOVED rcatwood 23/11/05 ptr to the interface of growing cell defined according to interface normal, for curvature calc only, xly, 2004/09/02 */ 
   int *nuclist;        /* ptr to array of grain boundary cell #  */




   
   CA_FLOAT *c_eqv_poly[NSOLMAX];  /*by Ludovic THUINET on 01-02-05*/
   CA_FLOAT *c_sol_poly[NSOLMAX]; /*by Ludovic THUINET on 01-02-05*/
   CA_FLOAT *c_fs_poly [NPHAMAX];/*by Ludovic THUINET on 05-05*/

   CA_FLOAT *c_fd;         /* ptr to array of cell state:            */
   CA_FLOAT *c_fs;         /* ptr to array of cell state:            */
   CA_FLOAT *c_fs_corrected;



   CA_FLOAT *c_nuc_thresh; /* cell nucleation threshold for grains     */

   int *nat_sol_site;   /*by THUINET 05/05*/
   int *nat_cell;       /*by THUINET 05/05*/
   int *nat_grain;      /* */


   CA_FLOAT *c_sol;        /* ptr to array of cell solute conc   for GAS    */
   CA_FLOAT *c_sol_alloy;  /* only used if PHASE DIAGRAM mode is on  */
   CA_FLOAT *c_eqv_alloy;  /* C_E:=C_S_fs+C_L*(1-fs)*/ /*by Wei WANG on 11-07-02*/

   CA_FLOAT *c_temp;       /* cell temperature */ /*by Wei WANG on 11-07-02*/
   CA_FLOAT *c_fg_temp;   /* cell temperature at macrostep 2d*/
   CA_FLOAT *c_fg_temp_next;  /* cell temperature at the next macro step*/
   CA_FLOAT *curv;         /* interface curvature */ /*by Wei WANG on 11-07-02*/
   CA_FLOAT *norm_x;       /* interface normal along x direction xly 2004/08/09 */ 
   CA_FLOAT *norm_y;       /* interface normal along y direction xly 2004/08/09 */
   CA_FLOAT *norm_z;       /* interface normal along z direction dn00 2005/10/18 */
   CA_FLOAT *dc_d;         /* decentred octahedron half diagonal */
   CA_FLOAT *dc_x;         /* centre x-coordinate */
   CA_FLOAT *dc_y;         /* centre y-coordinate */
   CA_FLOAT *dc_z;         /* centre z-coordinate */
   CA_FLOAT *cell_dfs_eutectic;
   CA_FLOAT *cell_dfs_primary;
   CA_FLOAT *fs_b_eut;
   CA_FLOAT *fs_b_eut_old;
   CA_FLOAT *fs_n_eut;
   CA_FLOAT *fs_n_eut_old;
   CA_FLOAT *fs_t_eut;
   CA_FLOAT *sch_fs;       /* Only used if SCHIEL mode is on          */
} SB_struct;

#endif /* SUBBLOCK_H */
