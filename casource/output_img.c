
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
/*  output.c:                     */
/* All subroutines related to output.                           */
/* including:                     */
/*   write_slice:       Write out an image slice from a sb      */
/*   write_bb_bin:      Write entire bigblock as binary image   */
/*                      can be viewed with pdl image program    */
/*   write_slice_conc:       Write out an image slice from a sb */
/*                           showing solute conc                */
/*   write_bb_bin_conc:      Write entire bigblock as binary image */
/*                           showing solute concentration       */
/*                      can be viewed with pdl image program    */
/****************************************************************/
/* Written by Peter D. Lee & Robert C. Atwood, Imperial College */
/* Wed Jul  1 18:38:31 bst 1998                       */
/****************************************************************/
/*    MODIFIED by:                  */
/*  PDL: sept 5, 1998                  */
/* RCA thu june 24 14:38:31 bst 1999                       */
/****************************************************************/
/****** To Do List **********************************************/
/* 1) figure out better colour scheme for conc.                 */
/****************************************************************/
/*RCS Id:$Id: output_img.c 1356 2008-08-18 13:41:15Z  $*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PNG_OUTPUT
#include "gd.h"
#endif

#include <string.h>

#include "machine.h"
#include "matprops.h"
#include "blocks.h"
#include "colour.h"
#include "interp.h"
#include "props.h"
#include "pore_routines.h"
/* functions defined later in file  */

/********************************************************/
/********************************************************/
/*      Subroutine to write entire bigblock as a   */
/* binary image (read with pdl image program      */
/* testflag zero for normal operation, number to create testfile*/
/********************************************************/
/********************************************************/
int write_bb_bin (BB_struct * bp, int testflag)
{
  char command[MAX_STRING_LEN];
  int nsbx, nsby, nsbz;         /* tmp counters */
  int bz, cz, by, cy, bx, cx;   /* tmp counters */
  int sbnum, cnum, ncx, ncy, ncz;       /* tmp counters */
  FILE *fp;                     /* tmp filehandle */
  int *gr;
  char *ip, *imgdata;
  char bsize[3];                /* x y and z size of block */

   /************************************************/
  /* open output file to write binary image to   */
   /************************************************/
  if (bp->ctrl->external == 0) {
    sprintf (command, "BL_%st%09d.bin", bp->ctrl->fn_base, bp->step);
  } else {
    sprintf (command, "BL_%st%09d.bin", bp->ctrl->fn_base, bp->step);
  }
  if ((fp = fopen (command, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file [%s]\n", command);
    return (1);
  }

  nsbx = bp->nsb[0];
  nsby = bp->nsb[1];
  nsbz = bp->nsb[2];
  ncx = bp->nc[0];
  ncy = bp->nc[1];
  ncz = bp->nc[2];
  bsize[0] = (char) ncx;
  bsize[1] = (char) ncy;
  bsize[2] = (char) ncz;
  if (!(imgdata = (char *) calloc (1 + ncx, sizeof (char)))) {
    fprintf (stderr, "image: pdata malloc failed\n");
    return 1;
  }

  sbnum = 0;

  fwrite (bsize, sizeof (char), 3, fp); /* prepend volume size as used by avs */
  /*one byte each for x,y,and z information */
  for (bz = 0; bz < nsbz; bz++) {       /* loop sb's in z direction */
    for (cz = 0; cz < ncz; cz++) {      /* loop cells's in z direction */
      for (by = 0; by < nsby; by++) {   /* loop sb's in y direction */
        for (cy = 0; cy < ncy; cy++) {  /* loop cell's in y direction */
          for (bx = 0; bx < nsbx; bx++) {       /* loop sb's in x direction */

            sbnum = bx + by * nsbx + bz * (nsbx * nsby);
            gr = bp->sb[sbnum]->gr;
            ip = imgdata;
            cnum = cy * ncx + cz * (ncx * ncy);

            if (bp->sb[sbnum]->open == TRUE) {
              for (cx = 0; cx < ncx; cx++) {    /* through converting to char */
                *(ip++) = (char) (1 + ((*(gr + cnum + cx))) % 253);
              }
            } else {
              for (cx = 0; cx < ncx; cx++) {    /* inactive sb set to 0 */
                *(ip++) = (char) (0);
              }
            }

            fwrite (imgdata, sizeof (char), ncx, fp);

          }                     /* sb's in x */
        }                       /* cell's in y */
      }                         /* sb's in y */
    }                           /* cells's in z */
  }                             /* sb's in z */

  free (imgdata);
  fclose (fp);
  return (0);
}                               /* end of write_bb_bin subroutine */

int write_bb_conc (BB_struct * bp, Value_struct * val)
{
#ifdef JUNK
  fprintf (stderr, "Sorrry, write_bb_conc not fixed up yet. Returning to regularly scheduled program...\n");
  return (0);
#endif /*JUNK*/
  char command[MAX_STRING_LEN];
  int nsbx, nsby, nsbz;         /* tmp counters */
  int bz, cz, by, cy, bx, cx;   /* tmp counters */
  int sbnum, cnum, ncx, ncy, ncz;       /* tmp counters */
  CA_FLOAT tmp;                 /* tmp CA_FLOAT var. */
  FILE *fp;                     /* tmp filehandle */
  CA_FLOAT *info;
  char *ip, *imgdata;
  char bsize[3];                /* x y and z size of block */

   /************************************************/
  /* open output file to write binary image to   */
  /* fixed filename for now         */
   /************************************************/
  if (bp->ctrl->external == 0) {
    sprintf (command, "BL_%s%st%09d.bin", val->id_string, bp->ctrl->fn_base, bp->step);
  } else {
    sprintf (command, "BL_%s%st%09d.bin", val->id_string, bp->ctrl->fn_base, bp->step);
  }
  if ((fp = fopen (command, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file [%s]\n", command);
    return (1);
  }
  nsbx = bp->nsb[0];
  nsby = bp->nsb[1];
  nsbz = bp->nsb[2];
  ncx = bp->nc[0];
  ncy = bp->nc[1];
  ncz = bp->nc[2];
  bsize[0] = (char) ncx;
  bsize[1] = (char) ncy;
  bsize[2] = (char) ncz;
  if (!(imgdata = (char *) calloc (ncx + 1, sizeof (char)))) {
    fprintf (stderr, "image: pdata malloc failed\n");
    return 1;
  }

  fwrite (bsize, sizeof (char), 3, fp); /* prepend volume size as used by avs */
  /*one byte each for x,y,and z information */
  sbnum = 0;
  for (bz = 0; bz < nsbz; bz++) {       /* loop sb's in z direction */
    for (cz = 0; cz < ncz; cz++) {      /* loop cells's in z direction */
      for (by = 0; by < nsby; by++) {   /* loop sb's in y direction */
        for (cy = 0; cy < ncy; cy++) {  /* loop cell's in y direction */
          for (bx = 0; bx < nsbx; bx++) {       /* loop sb's in x direction */

            sbnum = bx + by * nsbx + bz * (nsbx * nsby);
            info = (val->block_array[sbnum]);
            ip = imgdata;
            cnum = cy * ncx + cz * (ncx * ncy);

            for (cx = 0; cx < ncx; cx++) {      /* through converting to char */
              if (bp->ctrl->diff_log_disp == TRUE)
                *(ip++) = (char) (10 + (int) (LOGP ((*(info + cnum + cx)))
                                              * 100) % 245);
              else
                *(ip++) = (char) (10 + (int) ((*(info + cnum + cx)) * 100) % 245);
            }

            fwrite (imgdata, sizeof (char), ncx, fp);

          }                     /* sb's in x */
        }                       /* cell's in y */
      }                         /* sb's in y */
    }                           /* cells's in z */
  }                             /* sb's in z */

  free (imgdata);
  fclose (fp);
  return (0);
}                               /* end of write_bb_conc subroutine */

int pore_write (BB_struct * bp)
{
  char command[MAX_STRING_LEN];
  int nsbx, nsby, nsbz;         /* tmp counters */
  int compflag;
  int bz, cz, by, cy, bx, cx;   /* tmp counters */
  int sbnum, cnum, ncx, ncy, ncz;       /* tmp counters */
  FILE *fp;                     /* tmp filehandle */
  char *ip, *imgdata;
  char bsize[3];                /* x y and z size of block */
  PORE_str *c_p;
  p_c_node *node;
  int pore;

   /************************************************/
  /* open output file to write binary image to   */
  /* fixed filename for now         */
   /************************************************/
  sprintf (command, "BL_P_%s%st%06d.bin", "ONLY", bp->ctrl->fn_base, bp->step);
  if ((fp = fopen (command, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file [%s]\n", command);
    return (1);
  }

  nsbx = bp->nsb[0];
  nsby = bp->nsb[1];
  nsbz = bp->nsb[2];
  ncx = bp->nc[0];
  ncy = bp->nc[1];
  ncz = bp->nc[2];
  bsize[0] = (char) ncx;
  bsize[1] = (char) ncy;
  bsize[2] = (char) ncz;
  if (!(imgdata = (char *) calloc (ncx + 1, sizeof (char)))) {
    fprintf (stderr, "image: pdata malloc failed\n");
    return 1;
  }

  fwrite (bsize, sizeof (char), 3, fp); /* prepend volume size as used by avs */
  /*one byte each for x,y,and z information */
  sbnum = 0;
  for (bz = 0; bz < nsbz; bz++) {       /* loop sb's in z direction */
    for (cz = 0; cz < ncz; cz++) {      /* loop cells's in z direction */
      for (by = 0; by < nsby; by++) {   /* loop sb's in y direction */
        for (cy = 0; cy < ncy; cy++) {  /* loop cell's in y direction */
          for (bx = 0; bx < nsbx; bx++) {       /* loop sb's in x direction */

            sbnum = bx + by * nsbx + bz * (nsbx * nsby);
            ip = imgdata;
            cnum = cy * ncx + cz * (ncx * ncy);

            for (cx = 0; cx < ncx; cx++) {      /* through converting to char */
              if (bp->sb[sbnum]->open != SB_OPEN) {
                *(ip++) = (char) (1);
              } else {
                *(ip++) = (char) (255);
              }
            }
            c_p = bp->sb[sbnum]->porelist;
            /* *sigh* */
            /* loop thru every pore finding elements in this row */
            /* ohterwise need to copy entire big block first and then do all pores */

            if (bp->sb[sbnum]->open != SB_OPEN) {

            } else {
              for (pore = 0; pore < bp->sb[sbnum]->Npores; pore++, c_p++) {
                switch (c_p->State) {
                case PORE_FROZEN:
                case PORE_MULTI:
                case PORE_TUBE:
                case PORE_SPHERE:
                  for (node = c_p->boundary->first; node != NULL; node = node->next) {
                    if (node->Cell[1] == cy && node->Cell[2] == cz) {
                      *(imgdata + node->Cell[0]) = (char) (0);
                    }
                  }
                  break;
                case PORE_NONE:
                case PORE_OFF:
                case PORE_LATENT:
                  break;
                default:
                  break;
                }
              }
            }                   /* end sb open check */

            fwrite (imgdata, sizeof (char), ncx, fp);

          }                     /* sb's in x */
        }                       /* cell's in y */
      }                         /* sb's in y */
    }                           /* cells's in z */
  }                             /* sb's in z */

  free (imgdata);
  fclose (fp);
  return (0);
}                               /* end of pore_write subroutine */

#ifdef JUNK
/* same as above but include pores somehow*/
int pore_write_bb_conc (BB_struct * bp, Value_struct * val)
{
  char command[MAX_STRING_LEN];
  int nsbx, nsby, nsbz;         /* tmp counters */
  int bz, cz, by, cy, bx, cx;   /* tmp counters */
  int sbnum, cnum, ncx, ncy, ncz;       /* tmp counters */
  CA_FLOAT tmp;                 /* tmp CA_FLOAT var. */
  FILE *fp;                     /* tmp filehandle */
  CA_FLOAT *info;
  char *ip, *imgdata;
  char bsize[3];                /* x y and z size of block */
  PORE_str *c_p;
  p_c_node *node;
  int pore;

   /************************************************/
  /* open output file to write binary image to   */
  /* fixed filename for now         */
   /************************************************/
  if (bp->ctrl->external == 0) {
    sprintf (command, "BL_P_%s%st%09d.bin", val->id_string, bp->ctrl->fn_base, bp->step);
  } else {
    sprintf (command, "BL_P_%s%st%09d.bin", val->id_string, bp->ctrl->fn_base, bp->step);
  }
  if ((fp = fopen (command, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file [%s]\n", command);
    return (1);
  }
  nsbx = bp->nsb[0];
  nsby = bp->nsb[1];
  nsbz = bp->nsb[2];
  ncx = bp->nc[0];
  ncy = bp->nc[1];
  ncz = bp->nc[2];
  bsize[0] = (char) ncx;
  bsize[1] = (char) ncy;
  bsize[2] = (char) ncz;
  if (!(imgdata = (char *) calloc (ncx + 1, sizeof (char)))) {
    fprintf (stderr, "image: pdata malloc failed\n");
    return 1;
  }

  fwrite (bsize, sizeof (char), 3, fp); /* prepend volume size as used by avs */
  /*one byte each for x,y,and z information */
  sbnum = 0;
  for (bz = 0; bz < nsbz; bz++) {       /* loop sb's in z direction */
    for (cz = 0; cz < ncz; cz++) {      /* loop cells's in z direction */
      for (by = 0; by < nsby; by++) {   /* loop sb's in y direction */
        for (cy = 0; cy < ncy; cy++) {  /* loop cell's in y direction */
          for (bx = 0; bx < nsbx; bx++) {       /* loop sb's in x direction */

            sbnum = bx + by * nsbx + bz * (nsbx * nsby);
            info = (val->block_array[sbnum]);
            ip = imgdata;
            cnum = cy * ncx + cz * (ncx * ncy);

            for (cx = 0; cx < ncx; cx++) {      /* through converting to char */
              if (!(bp->sb[sbnum]->open)) {
                *(ip++) = (char) (1);
              } else {
                *(ip++) = (char) (10 + (int) ((*(info + cnum + cx)) * 100) % 245);
              }
            }
            c_p = bp->sb[sbnum]->porelist;
            /* *sigh* */
            /* loop thru every pore finding elements in this row */
            /* ohterwise need to copy entire big block first and then do all pores */

            for (pore = 0; pore < bp->sb[sbnum]->Npores; pore++, c_p++) {
              switch (c_p->State) {
              case PORE_FROZEN:
              case PORE_MULTI:
              case PORE_TUBE:
              case PORE_SPHERE:
                for (node = c_p->boundary->first; node != NULL; node = node->next) {
                  if (node->Cell[1] == cy && node->Cell[2] == cz) {
                    *(imgdata + node->Cell[0]) = (char) (0);
                  }
                }
                break;
              case PORE_NONE:
              case PORE_OFF:
              case PORE_LATENT:
                break;
              default:
                break;
              }
            }

            fwrite (imgdata, sizeof (char), ncx, fp);

          }                     /* sb's in x */
        }                       /* cell's in y */
      }                         /* sb's in y */
    }                           /* cells's in z */
  }                             /* sb's in z */

  free (imgdata);
  fclose (fp);
  return (0);
}                               /* end of write_bb_conc subroutine */
#endif /*JUNK*/
int write_slice_sat (BB_struct * bp, int sbnum, int slice)
{
  SB_struct *sp;
  Ctrl_str *cp;
  FILE *SS_fd;
  char filename[MAX_STRING_LEN];
  int nxny, rsize, nx, ny;
  CA_FLOAT *ssp, *ss, *c_temp_p, *fs, *alloy, *gas;
  int i, j, index;
  CA_FLOAT **conc_multi;

  cp = bp->ctrl;
  if (!(cp->diffuse && cp->diffuse_alloy))
    return (0);

  if (bp->sb[sbnum]->open == FALSE)
    return (0);

  fprintf (stderr, "write_supersat slice [sb:%d/%d]\n", sbnum, slice);
  sp = bp->sb[sbnum];
  nx = bp->nc[0];
  ny = bp->nc[1];
  nxny = nx * ny;

  if (slice >= bp->nc[2]) {
    fprintf (stderr, "write_supersat slice [sb:%d/%d] does not exist!\n", sbnum, slice);
    return (1);
  }
  fs = sp->c_fs + slice * nxny;
  alloy = sp->c_sol_alloy + slice * nxny;
  c_temp_p = sp->c_temp;
  index = slice * nxny;
  gas = sp->c_sol + slice * nxny;
  if (bp->ctrl->external == 0) {
    sprintf (filename, "F_SS_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  } else {
    sprintf (filename, "F_SS_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  }
  if ((SS_fd = fopen (filename, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file %s\n", filename);
    return (1);
  } else {
    ss = (CA_FLOAT *) malloc ((nxny + 1) * sizeof (CA_FLOAT));
    ssp = ss;
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
        *ssp = *gas / (find_sat (&(bp->mprops), *c_temp_p + STD_TEMP, *alloy, *fs));
        c_temp_p++;
        ssp++;
        fs++;
        alloy++;
        gas++;
      }
    }

    rsize = fwrite (ss, sizeof (CA_FLOAT), nxny, SS_fd);
    fclose (SS_fd);
  }
  free (ss);
  return (0);
}

/**************************************************/
/* write a slice of the undercooling at each cell */
/**************************************************/
int write_slice_undercool (BB_struct * bp, int sbnum, int slice)
{
  SB_struct *sp;
  Ctrl_str *cp;
  FILE *fp;
  char filename[MAX_STRING_LEN];
  int nxny, rsize, nx, ny;
  CA_FLOAT *ucp, *uc, *c_temp_p, *alloy, *fs;
  CA_FLOAT Tliq, Tunder, umax, umin;
  int i, j, index, imax, imin, jmax, jmin;
  int flag = 0;
  int ele_num, ele_1;

  cp = bp->ctrl;

  ele_num = cp->NUM_COMP;       /* number of elements in the alloy */
  ele_1 = ele_num - 1;



  /* silently ignore for these options */
  if (!(cp->diffuse_alloy))
    return (0);

  if (bp->sb[sbnum]->open == FALSE)
    return (0);

  if (cp->diffuse_alloy_multi == TRUE)
    return (0);

  fprintf (stderr, "write_slice_undercool[sb:%d/%d]\n", sbnum, slice);
  sp = bp->sb[sbnum];
  nx = bp->nc[0];
  ny = bp->nc[1];
  nxny = nx * ny;

  /* check for some inconsistent conditions */
  /* bad slice number */
  if (slice >= bp->nc[2]) {
    fprintf (stderr, "WARNING: write_slice_undercool [sb:%d/%d] does not exist!\n", sbnum, slice);
    return (1);
  }
  /* bad option */
  if (!(bp->ctrl->phase_diag_on)) {
    fprintf (stderr, "WARNING: write_slice_undercool: not used without phase_diag mode.\n");
    return (1);
  }

  /* find the desired slice */
  alloy = sp->c_sol_alloy + slice * nxny;
  fs = sp->c_fs + slice * nxny;
  c_temp_p = sp->c_temp;
  index = slice * nxny;

  sprintf (filename, "F_UC_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  fp = fopen (filename, "w");
  ucp = uc = (CA_FLOAT *) malloc ((nxny + 1) * sizeof (CA_FLOAT));
  for (j = 0; j < ny; j++) {
    for (i = 0; i < nx; i++) {
      if (*fs == NOT_CASTING) {
        *ucp = 0;
      } else {
        /* find the liquidus and then the undercooling */
        if (bp->ctrl->diffuse_alloy == TRUE){
          Tliq = cell_liq_calc (*alloy, &(bp->mprops));
        }
        if (bp->ctrl->diffuse_alloy_poly == TRUE){
          Tliq = cell_liq_calc_poly (*alloy, &(bp->mprops),ele_1);
        }
        Tunder = Tliq - *c_temp_p;
        *ucp = Tunder;

        /* start off the min/max at the first nonmould cell */
        if (flag == 0) {
          umax = umin = Tunder;
          imax = imin = i;
          jmax = jmin = j;
          flag = 1;
        }

        /* store the min/max value and location */
        if (Tunder > umax) {
          umax = Tunder;
          imax = i;
          jmax = j;
        }
        if (Tunder < umin) {
          umin = Tunder;
          imin = i;
          jmin = j;
        }

      }
      c_temp_p++;
      ucp++;
      alloy++;
      fs++;
    }
  }

  rsize = fwrite (uc, sizeof (CA_FLOAT), nxny, fp);
  fclose (fp);
  free (uc);

  fprintf (stderr, "write_slice_undercool: umax[%i,%i],%.5g, umin[%i,%i],%.5g\n", imax, jmax, umax, imin, jmin, umin);
  return (0);
}

/*******************************************/
/* end of write_slice_undercool ************/
/*******************************************/

/** \todo  eliminate this call -- general -- easy */
int write_slice_diff (BB_struct * bp, int sbnum, int slice)
{
  return (0);
}

int write_slice_curv (BB_struct * bp, int sbnum, int slice)
{
  SB_struct *sp;
  Ctrl_str *cp;
  FILE *FS_fd, *FS_fdb, *FS_fdt, *FS_fdneut;
  char filename[MAX_STRING_LEN];
  int nxny, rsize;
  CA_FLOAT *fs, *fsb, *fst, *fsneut;

  if (bp->sb[sbnum]->open == FALSE)
    return (0);

  fprintf (stderr, "write_cu_conc slice [sb:%d/%d]\n", sbnum, slice);
  cp = bp->ctrl;
  sp = bp->sb[sbnum];
  nxny = bp->nc[0] * bp->nc[1];

  if (slice >= bp->nc[2]) {
    fprintf (stderr, "write_cu_conc slice [sb:%d/%d] does not exist!\n", sbnum, slice);
    return (1);
  }
  fs = sp->curv + slice * nxny;
  if (bp->ctrl->diffuse_alloy_multi == TRUE) {
    fsb = sp->fs_b_eut + slice * nxny;
    fst = sp->fs_t_eut + slice * nxny;
    fsneut = sp->fs_n_eut + slice * nxny;
  }
  if (bp->ctrl->external == 0) {
    sprintf (filename, "F_CU_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  } else {
    sprintf (filename, "F_CU_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  }
  if ((FS_fd = fopen (filename, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file %s\n", filename);
    return (1);
  } else {
    rsize = fwrite (fs, sizeof (CA_FLOAT), nxny, FS_fd);
    fclose (FS_fd);
  }
  if (bp->ctrl->diffuse_alloy_multi) {

    sprintf (filename, "F_CU_B_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
    if ((FS_fdb = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output file %s\n", filename);
      return (1);
    } else {
      rsize = fwrite (fsb, sizeof (CA_FLOAT), nxny, FS_fdb);
      fclose (FS_fdb);
    }

    sprintf (filename, "F_CU_T_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
    if ((FS_fdt = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output file %s\n", filename);
      return (1);
    } else {
      rsize = fwrite (fst, sizeof (CA_FLOAT), nxny, FS_fdt);
      fclose (FS_fdt);
    }

    sprintf (filename, "F_FS_N_EUT_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
    if ((FS_fdneut = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output file %s\n", filename);
      return (1);
    } else {
      rsize = fwrite (fsneut, sizeof (CA_FLOAT), nxny, FS_fdneut);
      fclose (FS_fdneut);
    }

  }
  return (0);
}

int write_slice_fs (BB_struct * bp, int sbnum, int slice)
{
  SB_struct *sp;
  Ctrl_str *cp;
  FILE *FS_fd, *FS_fdb, *FS_fdt, *FS_fdneut;
  char filename[MAX_STRING_LEN];
  int nxny, rsize;
  CA_FLOAT *fs, *fsb, *fst, *fsneut;

  if (bp->sb[sbnum]->open == FALSE)
    return (0);

  fprintf (stderr, "write_fs_conc slice [sb:%d/%d]\n", sbnum, slice);
  cp = bp->ctrl;
  sp = bp->sb[sbnum];
  nxny = bp->nc[0] * bp->nc[1];

  if (slice >= bp->nc[2]) {
    fprintf (stderr, "write_fs_conc slice [sb:%d/%d] does not exist!\n", sbnum, slice);
    return (1);
  }
  fs = sp->c_fs + slice * nxny;
  if (bp->ctrl->diffuse_alloy_multi == TRUE) {
    fsb = sp->fs_b_eut + slice * nxny;
    fst = sp->fs_t_eut + slice * nxny;
    fsneut = sp->fs_n_eut + slice * nxny;
  }
  if (bp->ctrl->external == 0) {
    sprintf (filename, "F_FS_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  } else {
    sprintf (filename, "F_FS_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
  }
  if ((FS_fd = fopen (filename, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file %s\n", filename);
    return (1);
  } else {
    rsize = fwrite (fs, sizeof (CA_FLOAT), nxny, FS_fd);
    fclose (FS_fd);
  }
  if (bp->ctrl->diffuse_alloy_multi) {

    sprintf (filename, "F_FS_B_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
    if ((FS_fdb = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output file %s\n", filename);
      return (1);
    } else {
      rsize = fwrite (fsb, sizeof (CA_FLOAT), nxny, FS_fdb);
      fclose (FS_fdb);
    }

    sprintf (filename, "F_FS_T_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
    if ((FS_fdt = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output file %s\n", filename);
      return (1);
    } else {
      rsize = fwrite (fst, sizeof (CA_FLOAT), nxny, FS_fdt);
      fclose (FS_fdt);
    }

    sprintf (filename, "F_FS_N_EUT_%s_sb%01ds%01dt%09d.bin", cp->fn_base, sbnum, slice, bp->step);
    if ((FS_fdneut = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output file %s\n", filename);
      return (1);
    } else {
      rsize = fwrite (fsneut, sizeof (CA_FLOAT), nxny, FS_fdneut);
      fclose (FS_fdneut);
    }

  }
  return (0);
}

void write_sb_slice_temp (BB_struct * bp, int sbnum, int slice)
{
  int i, j, n;
  CA_FLOAT *temp_p;
  int *gr_p;
  char filename[MAX_STRING_LEN];
  FILE *fd, *fd_change;
  int nxny;
  int slice_start;
  int inttemp;
  CA_FLOAT mintemp, maxtemp, tempdiffinv;
#ifdef PNG_OUTPUT
  int pngcolours[33];
  gdImagePtr im_out;
#endif

#ifdef VERBOSE_EXTERNAL
  float *deltat;
#endif /*VERBOSE_EXTERNAL */

  if (slice >= bp->nc[2]) {
    fprintf (stderr, "WARNING: write_sb_slice_temp: slice %i out of range \n", slice);
    return;
  }
  if (bp->sb[sbnum]->open == FALSE) {
    return;
  }


  nxny = bp->nc[0] * bp->nc[1];
  slice_start = slice * nxny;
  temp_p = bp->sb[sbnum]->c_temp + slice_start;
  gr_p = bp->sb[sbnum]->gr + slice_start;

  if (bp->ctrl->floatdump) {
    sprintf (filename, "F_T_%s_sb%01ds%01dt%09d.bin\0", bp->ctrl->fn_base, sbnum, slice, bp->step);
    fd = fopen (filename, "w");
    fwrite (temp_p, sizeof (CA_FLOAT), nxny, fd);
    fclose (fd);
#ifdef VERBOSE_EXTERNAL
    /* output the stored temperature change amount for inteprolation */
    deltat = (float *) calloc (nxny, sizeof (float));
    for (i = 0; i < nxny; i++) {
      deltat[i] = *(bp->cell_temp_change_extern + slice_start + i) * bp->delt;
    }
    sprintf (filename, "F_TC_%s_sb%01ds%01dt%09d.bin\0", bp->ctrl->fn_base, sbnum, slice, bp->step);
    fd = fopen (filename, "w");
    fwrite (deltat, sizeof (float), nxny, fd);
    fclose (fd);
#endif /* VERBOSE_EXTERNAL */
  }


#ifdef PNG_OUTPUT
  im_out = gdImageCreate (bp->nc[0], bp->nc[1]);
  for (n = 0; n < 32; n++) {
    pngcolours[n] = gdImageColorAllocate (im_out, n * 8, n * 8, n * 8);
  }
  pngcolours[32] = gdImageColorAllocate (im_out, 255, 0, 0);
  mintemp = bp->sb[sbnum]->Tvals.Tmin;
  maxtemp = bp->sb[sbnum]->Tvals.Tmax;
  if (maxtemp - mintemp != 0) {
    tempdiffinv = 31 / (maxtemp - mintemp);
  } else {
    tempdiffinv = 1;
  }
  for (j = 0; j < bp->nc[1]; j++) {
    for (i = 0; i < bp->nc[0]; i++) {
      if (*gr_p == NOT_CASTING) {
        gdImageSetPixel (im_out, i, j, pngcolours[32]);
      } else {
        if (*temp_p < mintemp) {
          inttemp = 0;
        } else if (*temp_p >= maxtemp) {
          inttemp = 31;
        } else {
          inttemp = ((int) (FLOOR ((*temp_p - mintemp) * tempdiffinv))) % 31;
        }

        gdImageSetPixel (im_out, i, j, pngcolours[inttemp]);
      }
      temp_p++;
      gr_p++;
    }
  }

  gdImageInterlace (im_out, 1);
  sprintf (filename, "T_%s_sb%01ds%01dt%09d.png", bp->ctrl->fn_base, sbnum, slice, bp->step);
  fd = fopen (filename, "w");
  gdImagePng (im_out, fd);
  fclose (fd);
  gdImageDestroy (im_out);
#endif /* PNG_OUTPUT */

  return;
}

/********************************************************/
/********************************************************/
/* write_slice_temp_pres                                     */
/*      Subroutine to write out a slice from a sb showing temperature and pressure.      */
/********************************************************/
/********************************************************/

int write_slice_temp_pres (BB_struct * bp, int sbnum, int slice)
{
  int i, n, nn, index;          /* tmp counters */
  int nx, ny, nxny, rsizetemp, rsizepres, start;
  int rowflag = FALSE;
  float *soltemp, *solpres;
  CA_FLOAT size;
  CA_FLOAT k;
  char *idtemp, *idpres;
  int rtn_flag = 0;             /* error flag on return */
  char command[MAX_STRING_LEN], filename[MAX_STRING_LEN], F_filename_temp[MAX_STRING_LEN], F_filename_pres[MAX_STRING_LEN];
  FILE *fd, *F_fd_temp, *F_fd_pres;
  Ctrl_str *cp;

  idtemp = "Temp";
  idpres = "Pres";

  cp = bp->ctrl;
  nx = bp->nc[0];
  ny = bp->nc[1];
  nxny = nx * ny;

  /* Check that slice is in range!!! */
  if (slice >= bp->nc[2]) {
    fprintf (stderr, "write_slice_conc slice [sb:%d/%d] does not exist!\n", sbnum, slice);
    return (1);
  }

  soltemp = (bp->current_cell_temp + slice * nxny);
  solpres = (bp->current_cell_pres + slice * nxny);

  /*open float data output file */

  if (cp->floatdump) {          /* BinDump option in primary control file */
    if (bp->ctrl->external == 1) {
      sprintf (F_filename_temp, "F_%s%s_sb%01ds%01dt%09d.bin", idtemp, cp->fn_base, sbnum, slice, bp->step);
      sprintf (F_filename_pres, "F_%s%s_sb%01ds%01dt%09d.bin", idpres, cp->fn_base, sbnum, slice, bp->step);
    } else {
      return (1);
    }
    if ((F_fd_temp = fopen (F_filename_temp, "w")) == NULL || (F_fd_pres = fopen (F_filename_pres, "w")) == NULL) {
      fprintf (stderr, "Error: can't open output files %s and %s \n", F_filename_temp, F_filename_pres);
      return (1);
    } else {
      /* output the raw contents of the soltemp and solpres arrays to binary files */
      rsizetemp = fwrite (soltemp, sizeof (float), nxny, F_fd_temp);
      rsizepres = fwrite (solpres, sizeof (float), nxny, F_fd_pres);
      rtn_flag += fclose (F_fd_temp);
      rtn_flag += fclose (F_fd_pres);
    }
  }                             /* end of float dump */
  return (rtn_flag);
}

/**************end of write_slice_temp_pres routine**************/
/********************************************************/
/********************************************************/
/* write_slice_conc                                     */
/*      Subroutine to write out a slice from a sb showing sol conc.      */
/********************************************************/
/********************************************************/

int write_slice_conc (BB_struct * bp, Value_struct * out_values, int sbnum, int slice, int m)
{
  int rtn_flag = 0;             /* error flag on return */
  int nxny;
  int i;
  char F_filename[MAX_STRING_LEN];
  CA_FLOAT *sol, *fs;
  CA_FLOAT min = 0, max = 0;
  FILE *F_fd;
  int flag = 0;

  nxny = bp->nc[0] * bp->nc[1];
  sol = out_values->block_array[sbnum] + slice * nxny;
  fs = bp->sb[sbnum]->c_fs + slice * nxny;

  /*open float data output file */

  sprintf (F_filename, "F_%s%s_sb%01ds%01dt%09d.bin", out_values->id_string, bp->ctrl->fn_base, sbnum, slice, bp->step);
  if ((F_fd = fopen (F_filename, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file %s\n", F_filename);
    return (1);
  } else {
    /* output the raw contents of the sol array to a binary file */
    fwrite (sol, sizeof (CA_FLOAT), nxny, F_fd);
    fclose (F_fd);
  }
  for (i = 0; i < nxny; i++) {
    if (fs[i] != NOT_CASTING) {
      if (flag == 0) {
        min = max = sol[i];
        flag = 1;
      } else {
        min = MIN (sol[i], min);
        max = MAX (sol[i], max);
      }
    }
  }
  fprintf (stderr, "write_slice_conc:smin,smax,%.5g,%.5g\n", min, max);

  return (0);
}                               /* end of write_slice_concsubroutine */

/********************************************************/
/********************************************************/
/* write_slice                                     */
/*      Subroutine to write out a slice from a sb       */
/********************************************************/
/********************************************************/
int write_slice (BB_struct * bp, int sbnum, int slice)
{
  int i, j, n, nn, index;       /* tmp counters */
  int nxny, rsize;
  int *gr;
  short int *intgr;
  int rtn_flag = 0;             /* error flag on return */
  PORE_str *pl;
  char command[MAX_STRING_LEN], filename[MAX_STRING_LEN];
  FILE *fd;
  SB_struct *sp;
  char *imgdata;                /* pointer to binary image data         */

   /*******************PNG setup using the gdlib library***********/
#ifdef PNG_OUTPUT
  int pngcolours[32];

  gdImagePtr im_out;

  im_out = gdImageCreate (bp->nc[0], bp->nc[1]);
  for (n = 0; n < 32; n++) {
    pngcolours[n] = gdImageColorAllocate (im_out, colours[3 * n], colours[3 * n + 1], colours[3 * n + 2]);
  }
  pngcolours[31] = gdImageColorAllocate (im_out, 0, 0, 0);
#endif
   /**************end of PNG colour allocation******************/

  sp = bp->sb[sbnum];
  if (sp->open == FALSE)
    return (0);

  if (bp->ctrl->pore == TRUE)
    pl = bp->sb[sbnum]->porelist;

  /* Check that slice is in range!!! */
  if (sbnum >= bp->ntsb) {
    fprintf (stderr, "write_slice: subblock [sb:%d] does not exist!\n", sbnum);
    return (1);
  }
  if (slice >= bp->nc[2]) {
    fprintf (stderr, "write_slice: slice [sb:%d/%d] does not exist!\n", sbnum, slice);
    return (1);
  }

  nxny = bp->nc[0] * bp->nc[1];

  /********************RGB setup for SGI platforms******************/
  if (!(imgdata = (char *) calloc (3 * nxny + 10, sizeof (char)))) {
    fprintf (stderr, "image: pdata malloc failed\n");
    return (2);
  }

  if ((fd = fopen ("tmp.bin", "w")) == NULL) {
    fprintf (stderr, "Error: can't open input file [tmp.bin]\n");
    return (1);
  } else {
    if (bp->ctrl->external == 0) {
      sprintf (filename, "%s_sb%01ds%01dt%09d.rgb", bp->ctrl->fn_base, sbnum, slice, bp->step);
    } else {
      sprintf (filename, "%s_sb%01ds%01dt%09d.rgb", bp->ctrl->fn_base, sbnum, slice, bp->step);
    }
    nn = 0;
    gr = bp->sb[sbnum]->gr + slice * nxny;
    for (n = 0; n < nxny; n++) {
      if (*gr == 0) {           /*liquid */
        *(imgdata + nn) = colours[0];
        *(imgdata + nn + nxny) = colours[1];
        *(imgdata + nn + nxny * 2) = colours[2];
      } else if (*gr == EUT_GRAIN && bp->ctrl->show_eut) {      /*eutectic */
        *(imgdata + nn) = EUT_RED;
        *(imgdata + nn + nxny) = EUT_GREEN;
        *(imgdata + nn + nxny * 2) = EUT_BLUE;
      } else if (*gr == NOT_CASTING) {
        *(imgdata + nn) = 0;
        *(imgdata + nn + nxny) = 0;
        *(imgdata + nn + nxny * 2) = 0;
      } else {
        index = 3 * (1 + (*gr) % 31);
        *(imgdata + nn) = colours[index];
        *(imgdata + nn + nxny) = colours[index + 1];
        *(imgdata + nn + nxny * 2) = colours[index + 2];
      }
      gr++;
      nn++;
    }
  /**************PNG output for grain structure***************/
#ifdef PNG_OUTPUT
    gr = bp->sb[sbnum]->gr + slice * nxny;
    for (j = 0; j < bp->nc[1]; j++) {
      for (i = 0; i < bp->nc[0]; i++) {

        if (*gr == 0) {
          gdImageSetPixel (im_out, i, j, pngcolours[0]);
        } else if (*gr == NOT_CASTING) {
          gdImageSetPixel (im_out, i, j, pngcolours[31]);
        } else {
          index = 1 + (*gr) % 31;
          if (index == 31) {
            index = 30;
          }
          gdImageSetPixel (im_out, i, j, pngcolours[index]);
        }
        gr++;
      }
    }
#endif
  /******************end of PNG output for grain structure***********/

    if (bp->ctrl->pore == TRUE) {
      p_c_node *node;
      int ii;

      for (n = 0; n < sp->Npores; n++) {
        /* print pore if it exists ... */
#ifndef PRINT_ALL_PORES
        /* filter out pores nucleated too close to eutectic */
        /*from image */
        if (sp->porelist[n].NucTemp > (bp->mprops.alloyprops[0].T_eut + PORE_T_EUT_OFFSET))
#endif /*PRINT_ALL_PORES */
        {
          switch (sp->porelist[n].State) {
          case PORE_SPHERE:
          case PORE_FROZEN:
          case PORE_TUBE:
          case PORE_MULTI:
            for (node = sp->porelist[n].boundary->first; node != NULL; node = node->next) {
#ifndef XRAY_PORES
              if (node->Cell[2] == slice)
#endif
              {
                ii = node->Cell[0] + node->Cell[1] * bp->nc[0];
                *(imgdata + ii) = (char) (0);
                *(imgdata + ii + nxny) = (char) (0);
                *(imgdata + ii + nxny * 2) = (char) (0);
#ifdef PNG_OUTPUT
                index = 31;
                gdImageSetPixel (im_out, node->Cell[0], node->Cell[1], pngcolours[index]);
#endif
              }
            }
            break;
          case PORE_OFF:
          case PORE_NONE:
          case PORE_LATENT:
          case NOT_CASTING:
            /* do nothing */
            break;
          default:
            fprintf (stderr, "Error: output_img: write slice conc: PORE %i has left Earth orbit! State =  %i\n", n,
                     sp->porelist[n].State);
            break;
          }                     /*end test pore state */
        }                       /* end test pore nuc temp */
      }                         /*end loop through pores */
    }                           /*end if pore mode is true */
    rsize = fwrite (imgdata, sizeof (char), 3 * nxny, fd);
    fclose (fd);
    free (imgdata);

#ifdef PNG_OUTPUT
  /*********write out the PNG output************/
    gdImageInterlace (im_out, 1);
    sprintf (filename, "%s_sb%01ds%01dt%0.8d.png", bp->ctrl->fn_base, sbnum, slice, bp->step);
    fd = fopen (filename, "w");
    gdImagePng (im_out, fd);
    fclose (fd);
    /* write a copy for continuous update */
    fd = fopen ("step.png", "w");
    gdImagePng (im_out, fd);
    fclose (fd);

    gdImageDestroy (im_out);
#endif
    /* Save a slice of grain numbers to file as SHORT INT (16 bit) */
  }
  return (rtn_flag);
}                               /* end of write_slice subroutine */

void write_shortint_slice (BB_struct * bp, int sbnum, int slice)
{
  int nxny, n, rsize;
  static int external_first_flag = 0;
  char filename[MAX_STRING_LEN];
  FILE *fd;
  int *gr;
  unsigned short int *intgr;

  if (bp->sb[sbnum]->open != SB_OPEN)
    return;

  nxny = bp->nc[0] * bp->nc[1]; /* number of cells in a slice */
  sprintf (filename, "I_%s_sb%01ds%01dt%09d.bin", bp->ctrl->fn_base, sbnum, slice, bp->step);
  if ((fd = fopen (filename, "w")) == NULL) {
    fprintf (stderr, "ERROR: write_shortint_slice: can't open  file %s\n", filename);
    return;
  } else {
    gr = bp->sb[sbnum]->gr + slice * nxny;
    intgr = (unsigned short int *) malloc (sizeof (unsigned short int) * nxny + 1);
    for (n = 0; n < nxny; n++) {
      intgr[n] = (unsigned short int) gr[n];
    }

    rsize = fwrite (intgr, sizeof (unsigned short int), nxny, fd);
    fclose (fd);
  }
  free (intgr);
  /* for ProCast, create one slice of the element number */
  /* only on the first call to this routine */
  if (bp->ctrl->external && external_first_flag == 0) {
    external_first_flag = 1;
    sprintf (filename, "I_E_%s_sb%01ds%01dt%09d.bin", bp->ctrl->fn_base, sbnum, slice, bp->step);
    if ((fd = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "ERROR: write_float_slice: can't open  file %s\n", filename);
      return;
    } else {
      gr = bp->cell_element_array + slice * nxny;
      intgr = (unsigned short int *) malloc (sizeof (unsigned short int) * nxny + 1);
      for (n = 0; n < nxny; n++) {
        intgr[n] = (unsigned short int) gr[n];
      }

      rsize = fwrite (intgr, sizeof (unsigned short int), nxny, fd);
      fclose (fd);
    }
    free (intgr);
  }
}                               /* end of write_shortint_slice subroutine */

/********************************************************/
/********************************************************/
/* write_bb_slice                                     */
/*      Subroutine to write out a slice from a bigblock */
/* as an rgb format image                               */
/********************************************************/
/********************************************************/
int write_bb_slice (BB_struct * bp, int slice)
{
  char command[MAX_STRING_LEN], filename[MAX_STRING_LEN];
  int nsbx, nsby, nsbz, nxny;   /* tmp counters */
  int bz, cz, by, cy, bx, cx;   /* tmp counters */
  int sbnum, cnum, ncx, ncy, ncz, index;        /* tmp counters */
  FILE *fp;                     /* tmp filehandle */
  int *gr, *zerograin;
  char *ip, *imgdata, *r, *g, *b;

  r = bp->ctrl->rgbp->r;
  g = bp->ctrl->rgbp->g;
  b = bp->ctrl->rgbp->b;

  /*create zero list for inactive subblocks */
  if (!(zerograin = (int *) calloc (bp->ncsb + 1, sizeof (int)))) {
    fprintf (stderr, "ERROR: write_bb_slice- zerograin malloc failed\n");
    return (1);
  }

  /* Check that slice is in range!!! */
  if ((slice < 0) || (slice >= bp->tnc[2])) {
    fprintf (stderr, "write_bb_slice: slice [%d] does not exist!\n", slice);
    return (1);
  }

   /************************************************/
  /* open output file to write binary image to   */
  /* fixed filename for now         */
   /************************************************/
  sprintf (command, "tmp.bin");
  if ((fp = fopen (command, "w")) == NULL) {
    fprintf (stderr, "Error: can't open output file [%s]\n", command);
    return (1);
  }

  nsbx = bp->nsb[0];
  nsby = bp->nsb[1];
  nsbz = bp->nsb[2];
  ncx = bp->nc[0];
  ncy = bp->nc[1];
  ncz = bp->nc[2];
  sbnum = 0;
  nxny = bp->tnc[0] * bp->tnc[1];
  if (!(imgdata = (char *) calloc (3 * nxny + 1, sizeof (char)))) {
    fprintf (stderr, "ERROR: write_bb_slice- imgdata malloc failed\n");
    return 1;
  }

  bz = slice / bp->nc[2];
  cz = slice % bp->nc[2];
  ip = imgdata;

  switch (bp->ctrl->rgbmode) {

  case RGB_ORIENT:             /* use colour dependent upon the angle */
    {
      fprintf (stderr, "write_bb_slice: USING ORIENTED COLOURS\n");
      for (by = 0; by < nsby; by++) {   /* loop sb's in y direction   */
        for (cy = 0; cy < ncy; cy++) {  /* loop cell's in y direction */
          for (bx = 0; bx < nsbx; bx++) {       /* loop sb's in x direction   */

            sbnum = bx + by * nsbx + bz * (nsbx * nsby);
            if (bp->sb[sbnum]->open == TRUE) {
              gr = bp->sb[sbnum]->gr;
            } else {
              gr = zerograin;
            }
            cnum = cy * ncx + cz * (ncx * ncy);
            for (cx = 0; cx < ncx; cx++) {      /* loop along row within sb */
              if (*(gr + cnum + cx) == 0) {     /* liquid, set to black/white */
                *(ip) = colours[0];
                *(ip + nxny) = colours[1];
                *(ip + nxny * 2) = colours[2];
              } else {
                /* convert angle from rad to 0  -> 45 index */
                index = (bp->gr[*(gr + cnum + cx)]->dir_angle * 45.0 / PI_BY_FOUR + 44.9);
                if (index > RGB_SIZE) {
/* code folded from here */
                  fprintf (stderr, "ERROR: angle out of range [%d] .\n", index);
                  index = RGB_SIZE;
/* unfolding */
                } else if (index < 0) {
/* code folded from here */
                  fprintf (stderr, "ERROR: angle out of range [%d] .\n", index);
                  index = 0;
/* unfolding */
                }
                *(ip) = r[index];
                *(ip + nxny) = g[index];
                *(ip + nxny * 2) = b[index];
              }
              ip++;
            }

          }                     /* sb's in x */
        }                       /* cell's in y */
      }                         /* sb's in y */
    }
    break;                      /* end of case RGB_ORIENT */

  default:                     /* print error message and then fall through to RGB_random */
    fprintf (stderr, "ERROR: rgbmode value [%d] undefined.\n", bp->ctrl->rgbmode);
    /*fall through */
  case RGB_RANDOM:             /* use random colouring */
    for (by = 0; by < nsby; by++) {     /* loop sb's in y direction   */
      for (cy = 0; cy < ncy; cy++) {    /* loop cell's in y direction */
        for (bx = 0; bx < nsbx; bx++) { /* loop sb's in x direction   */

          sbnum = bx + by * nsbx + bz * (nsbx * nsby);
          if (bp->sb[sbnum]->open == TRUE) {
            gr = bp->sb[sbnum]->gr;
          } else {
            gr = zerograin;
          }
          cnum = cy * ncx + cz * (ncx * ncy);

          for (cx = 0; cx < ncx; cx++) {        /* through converting to char */
            if (*(gr + cnum + cx) == 0) {
              *(ip) = colours[0];
              *(ip + nxny) = colours[1];
              *(ip + nxny * 2) = colours[2];
            } else {
              index = 3 * (1 + (*(gr + cnum + cx)) % 31);
              *(ip) = colours[index];
              *(ip + nxny) = colours[index + 1];
              *(ip + nxny * 2) = colours[index + 2];
            }
            ip++;
          }

        }                       /* sb's in x */
      }                         /* cell's in y */
    }                           /* sb's in y */
    break;                      /* end of case RGB_RANDOM */

  }                             /* end of Flag switch */

  fwrite (imgdata, sizeof (char), 3 * nxny, fp);
  fclose (fp);
  free (imgdata);

  free (zerograin);
  return (0);
}                               /* end of write_bb_slice subroutine */

/********************************************************/
/********************************************************/
/* write_bb_T_slice                                     */
/*      Subroutine to write out a slice from a bigblock */
/* as an rgb format image                               */
/********************************************************/
/********************************************************/
int write_bb_T_slice (BB_struct * bp, int slice)
{
  char command[MAX_STRING_LEN], filename[MAX_STRING_LEN];
  int nsbx, nsby, nsbz, nxny;   /* tmp counters */
  int bz, cz, by, cy, bx, cx;   /* tmp counters */
  int sbnum, cnum, ncx, ncy, ncz, index;        /* tmp counters */
  int *ngr, i, j, k, ii, jj, kk;
  int n, nx, ny, nz;
  CA_FLOAT *tmp_ce, *out_ce, *nce;
  CA_FLOAT *tmp_undercooling, *p_undercooling, *p_temperature, *nal;
  CA_FLOAT Tliq, c_init, m_liq;
  CA_FLOAT tmp, Tmax, Tmin, T;  /* tmp CA_FLOAT var. */
  CA_FLOAT convert_r, convert_g, convert_b;     /* tmp CA_FLOAT var. */
  CA_FLOAT *tempdata, *tp;      /*temprature data array */
  FILE *fp;                     /* tmp filehandle */
  char *ip, *imgdata;

/*THUINET 22-02-05*/
  Ctrl_str *cp;
  int isol,ele_num,ele_1,iphs;
  CA_FLOAT *tmp_ce_poly, *out_ce_poly, *nce_poly;
  /*FIN THUINET 22-02-05*/
  /*THUINET 06-05*/
  CA_FLOAT *tmp_fs_poly, *out_fs_poly, *fs_poly;
  /*FIN THUINET 06-05*/

  /*THUINET 22-02-05*/
  cp=bp->ctrl;
  ele_num=cp->NUM_COMP;   /* number of elements in the alloy */
  ele_1=ele_num-1;
  /*FIN THUINET 22-02-05*/


   /*******************PNG setup using the gdlib library***********/
#ifdef PNG_OUTPUT
  int pngcolours[32];

  gdImagePtr im_out;

  im_out = gdImageCreate (bp->nc[0], bp->nc[1]);
  for (n = 0; n < 32; n++) {
    pngcolours[n] = gdImageColorAllocate (im_out, colours[3 * n], colours[3 * n + 1], colours[3 * n + 2]);
  }
#endif
   /**************end of PNG colour allocation******************/
  nx = bp->nc[0];
  ny = bp->nc[1];
  nz = bp->nc[2];
  nxny = nx * ny;

  /* Check that slice is in range!!! */
  if ((slice < 0) || (slice >= bp->tnc[2])) {
    fprintf (stderr, "ERROR: write_bb_T_slice: slice [%d] does not exist!\n", slice);
    return (1);
  }

   /************************************************/
  /* open output file to write binary image to   */
  /* fixed filename for now         */
   /************************************************/
  sprintf (command, "tmp.bin");
  if ((fp = fopen (command, "w")) == NULL) {
    fprintf (stderr, "ERROR: can't open output file [%s]\n", command);
    return (1);
  }

  if (bp->ctrl->fgrid_input) {
#ifdef JUNK
    Tmax = bp->fg->Tmax;
    Tmin = bp->fg->Tmin;
#endif
    Tmax = 2000;
    Tmin = 600;
  } else {
    Tmax = bp->mprops.Tliq;
    Tmin = bp->mprops.Tsol;
  }
  /* precalculate the conversion factors to rgb */
  convert_r = 255.0 / (Tmax - Tmin);
  convert_g = 255.0 / (Tmax - Tmin);
  convert_b = 255.0 / (Tmax - Tmin);

  nsbx = bp->nsb[0];
  nsby = bp->nsb[1];
  nsbz = bp->nsb[2];
  ncx = bp->nc[0];
  ncy = bp->nc[1];
  ncz = bp->nc[2];
  sbnum = 0;
  nxny = bp->tnc[0] * bp->tnc[1];

  /*allocate an array to store temperature data */
  if (bp->ctrl->floatdump) {
    if (!(tempdata = (CA_FLOAT *) calloc (nxny + 1, sizeof (CA_FLOAT)))) {
      fprintf (stderr, "ERROR: write_bb_slice- tempdata malloc failed\n");
      return 1;
    } else {
      tp = tempdata;
    }
  }

  /*allocate an array to store image data */
  if (!(imgdata = (char *) calloc (3 * nxny + 1, sizeof (char)))) {
    fprintf (stderr, "ERROR: write_bb_slice- imgdata malloc failed\n");
    return 1;
  }

  bz = slice / bp->nc[2];
  cz = slice % bp->nc[2];
  ip = imgdata;
   /**********************************************/
  /* loop through subblocks and cells for slice */
   /**********************************************/
  for (by = 0; by < nsby; by++) {       /* loop sb's in y direction   */
    for (cy = 0; cy < ncy; cy++) {      /* loop cells in y direction */
      for (bx = 0; bx < nsbx; bx++) {   /* loop sb's in x direction   */

        sbnum = bx + by * nsbx + bz * (nsbx * nsby);
        cnum = cy * ncx + cz * (ncx * ncy);

        for (cx = 0; cx < ncx; cx++) {  /* loop cells in x */
          i = cx + bx * ncx;
          j = cy + by * ncy;
          if (bp->sb[sbnum]->open == SB_OPEN) {
            T = bp->sb[sbnum]->c_temp[cnum + cx];
          } else {
            T = 0;
          }

          /*store temperature data in array if needed */
          if (bp->ctrl->floatdump) {
            *tp = T;
            tp++;
          }
          /*greyscale */
          *(ip) = (char) (T - Tmin) * convert_r;
          *(ip + nxny) = (char) (T - Tmin) * convert_g;
          *(ip + nxny * 2) = (char) (T - Tmin) * convert_b;
          if (T < Tmin) {
            *(ip) = (char) (255);
            *(ip + nxny) = (char) (0);
            *(ip + nxny * 2) = (char) (0);
          }
          if (T > Tmax) {
            *(ip) = (char) (0);
            *(ip + nxny) = (char) (0);
            *(ip + nxny * 2) = (char) (255);
          }
  /**************PNG output for temperature structure***************/
#ifdef PNG_OUTPUT
          if (T < Tmin) {
            gdImageSetPixel (im_out, i, j, pngcolours[0]);
          } else if (T > Tmax) {
            gdImageSetPixel (im_out, i, j, pngcolours[31]);
          } else {
            index = 1 + FLOOR ((T - Tmin) / (Tmax - Tmin) * 31);
            if (index == 31) {
              index = 30;
            }
            gdImageSetPixel (im_out, i, j, pngcolours[index]);
          }
#endif
  /******************end of PNG output for temperature structure***********/
          ip++;
        }                       /* cells in x */

      }                         /* sb's in x */
    }                           /* cells in y */
  }                             /* sb's in y */
   /**********************************************/
  /* end subblock and cell loop.                */
   /**********************************************/

  fwrite (imgdata, sizeof (char), 3 * nxny, fp);
  fclose (fp);
  free (imgdata);


#ifdef PNG_OUTPUT
  /*********write out the PNG output************/
  gdImageInterlace (im_out, 1);
  sprintf (filename, "T_%s_bb_s%01dt%09d.png", bp->ctrl->fn_base, slice, bp->step);
  fp = fopen (filename, "w");
  gdImagePng (im_out, fp);
  gdImageDestroy (im_out);
  fclose (fp);
#endif
  /*write out temperature float data */
  if (bp->ctrl->floatdump) {
    sprintf (filename, "F_T_%s_bb_s%01dt%09d.bin", bp->ctrl->fn_base, slice, bp->step);
    if ((fp = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "ERROR: can't open output file [%s]\n", command);
      return (1);
    }
    fwrite (tempdata, sizeof (CA_FLOAT), nxny, fp);
    fclose (fp);

    /*write out total undercooling by Wei Wang on 11-02-03,need improvement */
#ifdef JUNK
    if (!(tmp_undercooling = (CA_FLOAT *) calloc (ncx * ncy + 1, sizeof (CA_FLOAT)))) {
      fprintf (stderr, "ERROR: write_bb_slice- tmp_undercooling malloc failed\n");
      return 1;
    } else {
      p_undercooling = tmp_undercooling;
    }
    ngr = bp->sb[0]->gr;
    p_temperature = tempdata;   /* pointer to temperature */
    Tliq = bp->mprops.Tliq;

    for (k = 0; k < ncz; k++) {
      for (j = 0; j < ncy; j++) {
        for (i = 0; i < ncx; i++) {
          if (*ngr == NOT_CASTING) {
            *p_undercooling = -1000.;   /* -1000 for mould */
          } else {
            *p_undercooling = Tliq - *p_temperature;
          }
          ngr++;
          p_temperature++;
          p_undercooling++;
        }
      }
    }

    sprintf (filename, "F_tUC_%s_bb_s%01dt%06d.bin", bp->ctrl->fn_base, slice, bp->step);
    if ((fp = fopen (filename, "w")) == NULL) {
      fprintf (stderr, "ERROR: can't open output file [%s]\n", command);
      return (1);
    }
    fwrite (tmp_undercooling, sizeof (CA_FLOAT), ncx * ncy, fp);
    fclose (fp);

    /* output real  undercooling by Wei Wang on 11-02-03, need improvement */
    if (bp->ctrl->diffuse_alloy) {
      ngr = bp->sb[0]->gr;
      p_undercooling = tmp_undercooling;
      nal = bp->sb[0]->c_sol_alloy;     /* only one subblock considered */
      m_liq = bp->mprops.m_alloy;
      /*c_init = bp->mprops.gasprops.Cinit_alloy; */
      c_init = bp->mprops.alloyprops.Cinit;

      for (k = 0; k < ncz; k++) {
        for (j = 0; j < ncy; j++) {
          for (i = 0; i < ncx; i++) {
            if (*ngr == NOT_CASTING) {
              *p_undercooling = -1000.; /* -1000 for mould */
            } else if (*ngr > 0) {
              *p_undercooling = 0.;     /* 0 in solid */
            } else {
              *p_undercooling += m_liq * (*nal - c_init);
            }
            ngr++;
            nal++;
            p_undercooling++;
          }
        }
      }
      sprintf (filename, "F_rUC_%s_bb_s%01dt%06d.bin", bp->ctrl->fn_base, slice, bp->step);
      if ((fp = fopen (filename, "w")) == NULL) {
        fprintf (stderr, "ERROR: can't open output file [%s]\n", command);
        return (1);
      }
      fwrite (tmp_undercooling, sizeof (CA_FLOAT), ncx * ncy, fp);
      fclose (fp);
    }

    /* dump Equivalent concentration *//* by Wei WANG 11-02-03 */
    if (bp->ctrl->decentred_octahedron && bp->ctrl->diffuse_alloy) {
      nce = bp->sb[0]->c_eqv_alloy + slice * nxny;
      if (!(tmp_ce = (CA_FLOAT *) calloc (nxny + 1, sizeof (CA_FLOAT)))) {
        fprintf (stderr, "ERROR: write_bb_slice- tmp_ce malloc failed\n");
        return 1;
      } else {
        out_ce = tmp_ce;
      }
      ngr = bp->sb[0]->gr;

      for (kk = 0; kk < nz; kk++) {
        for (jj = 0; jj < ny; jj++) {
          for (ii = 0; ii < nx; ii++) {
            if (*ngr == NOT_CASTING) {
              *out_ce = -1.;     /*-1.for the mould*/
            } else {
              *out_ce = *nce;
            }
            ngr++;
            out_ce++;
            nce++;
          }
        }
      }

      sprintf (filename, "F_EQ_%s_sb%01ds%01dt%06d.bin", bp->ctrl->fn_base, sbnum, slice, bp->step);
      if ((fp = fopen (filename, "w")) == NULL) {
        fprintf (stderr, "Error: can't open output file %s\n", filename);
        return (1);
      } else {
        /* output the raw contents of the sol array to a binary file */
        fwrite (tmp_ce, sizeof (CA_FLOAT), nxny, fp);
        fclose (fp);
      }
      free (tmp_ce);
    }

    /* Equivalent concentration for polycomponent system */ /* by THUINET 22-02-05 */
    if (bp->ctrl->decentred_octahedron && bp->ctrl->diffuse_alloy_poly) {
      for (isol = 0; isol < ele_1; isol++){
        nce_poly = bp->sb[0]->c_eqv_poly[isol] + slice * nxny;
        if (!(tmp_ce_poly = (CA_FLOAT * ) calloc(nxny+1, sizeof(CA_FLOAT)))) {
          fprintf(stderr, "ERROR: write_bb_slice- tmp_ce_poly malloc failed\n");
          return 1;
        } else {
          out_ce_poly = tmp_ce_poly;
        }
        ngr = bp->sb[0]->gr;

        for(kk = 0; kk < nz; kk++) {
          for(jj = 0; jj < ny; jj++) {
            for(ii = 0; ii < nx; ii++) {
              if (*ngr == NOT_CASTING) {
                *out_ce_poly = -1.; /*-1.for the mould*/
              } else {
                *out_ce_poly = *nce_poly;

              if (*nce_poly < 0) {
                *nce_poly = *nce_poly;
              }

            }
            ngr ++;
            out_ce_poly ++;
            nce_poly ++;
          }
        }
      }

      sprintf(filename, "F_EQ_sol%1d_%s_sb%01ds%01dt%06d.bin",isol, bp->ctrl->fn_base, sbnum, slice, bp->step);
      if ((fp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Error: can't open output file %s\n", filename);
        return (1);
      } else {
        /* output the raw contents of the sol array to a binary file */
        fwrite (tmp_ce_poly, sizeof (CA_FLOAT), nxny, fp);
        fclose(fp);
      }
      free(tmp_ce_poly);
    }/*End loop on solute*/


    for (iphs = 0; iphs < bp->ctrl->NUM_PHS; iphs++){
      fs_poly = bp->sb[0]->c_fs_poly[iphs] + slice * nxny;
      if (!(tmp_fs_poly = (CA_FLOAT * ) calloc(nxny+1, sizeof(CA_FLOAT)))) {
        fprintf(stderr, "ERROR: write_bb_slice- tmp_fs_poly malloc failed\n");
        return 1;
      } else {
        out_fs_poly = tmp_fs_poly;
      }
      ngr = bp->sb[0]->gr;

      for(kk = 0; kk < nz; kk++) {
        for(jj = 0; jj < ny; jj++) {
          for(ii = 0; ii < nx; ii++) {

            if (*ngr == NOT_CASTING) {
              *out_fs_poly = -1.; /*-1.for the mould*/
            } else {
              *out_fs_poly = *fs_poly;
            }
            ngr ++;
            out_fs_poly ++;
            fs_poly ++;
          }
        }
      }

      sprintf(filename, "F_FS_solid%1d_%s_sb%01ds%01dt%06d.bin",iphs, bp->ctrl->fn_base, sbnum, slice, bp->step);
      if ((fp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Error: can't open output file %s\n", filename);
        return (1);
      } else {
        /* output the raw contents of the sol array to a binary file */
        fwrite (tmp_fs_poly, sizeof (CA_FLOAT), nxny, fp);
        fclose(fp);
      }
      free(tmp_fs_poly);
    }/*End loop on solids*/

    free (tmp_undercooling);
#endif
    free (tempdata);
  }
  return (0);
}                               /* end of write_bb_T_slice subroutine */

/********************************************************/
/********************************************************/
/* init_output_img                                      */
/*      Subroutine to init all arrays used in outputting*/
/* rgb format images                                    */
/********************************************************/
/* TO DO:                                               */
/* 1) add in reading of scale tie points and then       */
/*    generate the full scale.                          */
/********************************************************/
int init_output_img (RGB_struct * rgbp)
{
  int i;
  CA_FLOAT convert_r, convert_g, convert_b;     /* tmp CA_FLOAT var. */
  char *r, *g, *b;
  int size = RGB_SIZE;
  CA_FLOAT step;

  rgbp->size = size;
  /* malloc arrays to hold rgb colours */
  if (!(r = (char *) malloc (1 + size * sizeof (char)))) {
    fprintf (stderr, "ERROR: rgb array malloc failed\n");
    return (1);
  }
  if (!(g = (char *) malloc (1 + size * sizeof (char)))) {
    fprintf (stderr, "ERROR: rgb array malloc failed\n");
    return (1);
  }
  fprintf (stderr, "    alloc(cp->rgbp->g);\n");
  fprintf (stderr, " %x\n", (g));
  if (!(b = (char *) malloc (1 + size * sizeof (char)))) {
    fprintf (stderr, "ERROR: rgb array malloc failed\n");
    return (1);
  }

  if (rgbp->grey) {
    step = 250.0 / size;
    for (i = 0; i < size; i++) {
      r[i] = (char) (253 - i * step);
      g[i] = (char) (253 - i * step);
      b[i] = (char) (253 - i * step);
#ifdef JUNK
      if (i % 25 == 0) {
        r[i] = 255;
        g[i] = 0;
        b[i] = 0;
      }
#endif
    }
  } else {
    step = 220.0 / size;
    for (i = 0; i < size; i++) {
      r[i] = (char) (30 + (size - i) * step);
      g[i] = (char) 30;
      b[i] = (char) (30 + i * step);
    }
  }
  rgbp->r = r;
  rgbp->g = g;
  rgbp->b = b;

  return (0);
}                               /* end of init_output_img subroutine */

/********************************/
/* dump all pores in avs format */
/********************************/
/* dump all pores in avs format */
int avs_pores (BB_struct * bp, int sbnum)
{
/* dump all pores in avs format */

  int i, active = 0;
  int errors = 0;
  CA_FLOAT radcell;
  PORE_str *pl, *c_p;
  SB_struct *sp;
  p_c_node *node;
  char fld_filename[MAX_STRING_LEN], data_filename[MAX_STRING_LEN];
  FILE *fp, *dp;

  sp = bp->sb[sbnum];
  pl = sp->porelist;
  c_p = pl;                     /*current pore is zero */

  if (bp->ctrl->external == 0) {
    sprintf (data_filename, "VP_%s_sb%01dt%09d.val", bp->ctrl->fn_base, sbnum, bp->step);
    sprintf (fld_filename, "VP_%s_sb%01dt%09d.fld", bp->ctrl->fn_base, sbnum, bp->step);
  } else {
    sprintf (data_filename, "VP_%s_sb%01dt%09d.val", bp->ctrl->fn_base, sbnum, bp->step);
    sprintf (fld_filename, "VP_%s_sb%01dt%09d.fld", bp->ctrl->fn_base, sbnum, bp->step);
  }
  dp = fopen (data_filename, "w");

  for (i = 0; i < sp->Npores; i++) {
    switch (c_p->State) {
    case PORE_SPHERE:
    case PORE_MULTI:
    case PORE_TUBE:
    case PORE_FROZEN:
      for (node = c_p->boundary->first; node != NULL; node = node->next) {
        fprintf (dp, "%f %f %f %f\n", (float) (node->Cell[0]), (float) (node->Cell[1]), (float) (node->Cell[2]), 1.0);
        active++;
      }
      break;
    case PORE_NONE:            /*fall through */
    case PORE_LATENT:
    case PORE_OFF:
      /* do nuthin */
      break;
    default:
      break;
    }                           /* end of pore state switch */
    c_p++;                      /*update the Current Pore */
  }                             /* end of loop through pores */
  fclose (dp);

  fp = fopen (fld_filename, "w");

  fprintf (fp, "# AVS \n");
  fprintf (fp, "# file generated by umat_run\n");
  fprintf (fp, "# step %i\n", bp->step);
  fprintf (fp, "ndim=1\nnspace=3\n");
  fprintf (fp, "dim1=%i\n", active);
  fprintf (fp, "veclen=1\ndata=float\nfield=irregular\n");
  fprintf (fp, "coord 1 file=%s filetype=ascii offset=0 stride=4\n", data_filename);
  fprintf (fp, "coord 2 file=%s filetype=ascii offset=1 stride=4\n", data_filename);
  fprintf (fp, "coord 3 file=%s filetype=ascii offset=2 stride=4\n", data_filename);
  fprintf (fp, "variable 1 file=%s filetype=ascii offset=3 stride=4\n", data_filename);

  fclose (fp);
  return (errors);
}                               /*end of avs_pores */

/* Little subroutine to get rcs id into the object code */
/* so you can use ident on the compiled program  */
/* also you can call this to print out or include the rcs id in a file*/
char const *rcs_id_output_img_c ()
{
  static char const rcsid[] = "$Id: output_img.c 1356 2008-08-18 13:41:15Z  $";

  return (rcsid);
}

/* end of rcs_id_output_img_c subroutine */
/*
*/
