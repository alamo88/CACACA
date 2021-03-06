/* Structure definition for a csv format file stored in memory */
/*RCS Id:$Id: csv.h 887 2006-03-01 18:21:01Z rcatwood $*/
#ifndef CSV_H
#define CSV_H

#include "../machine.h"
#include "../constants.h"

typedef struct line_struct{
   int nperline; /* maximum allocated fields */
   int nfields; /* actual number of fields */
   #ifdef PC
   int ct;
   #endif
   char ** fields;
}LineData;


typedef struct csv_data {
   int nheaders;
   int nlines;
   int line_count;
   LineData ** headers;
   LineData ** lines;
} CsvData;

typedef struct csv_float {
   int nheaders;
   int nlines;
   int line_count;
   int nfields;
   LineData ** headers;
   CA_FLOAT ** data;
} CsvFloat;


typedef struct flag_struct {
   int i;
   int c;
   int o;
   int d;
   int V;
   int u;
   int t;
   int f;
   int nh;
   char ext[10];
} Flags;
#endif

/*
RCS Log:$Log$
RCS Log:Revision 11.1  2006/03/01 18:21:00  rcatwood
RCS Log:Merging polycomponent and gas with meltback
RCS Log:
RCS Log:Revision 10.2  2005/12/01 14:38:02  rcatwood
RCS Log:Merged xly_05 changes into the main trunk
RCS Log:Primarily involving melt-back
RCS Log:
RCS Log:Revision 10.1.2.2  2005/11/23 18:19:10  rcatwood
RCS Log:Result of merging mould_source and xly meltback+curvature 2d versions
RCS Log:
RCS Log:Revision 10.1  2005/11/03 11:56:48  rcatwood
RCS Log:New version number -- using mould_src as base
RCS Log:
RCS Log:Revision 1.1.4.2  2005/11/02 11:50:56  rcatwood
RCS Log:Read list files
RCS Log:
RCS Log:Revision 9.3  2003/12/10 17:36:23  kerman
RCS Log:added unit conversions & fixed for counting number of fields from the header
RCS Log:
RCS Log:Revision 1.1.2.3  2003/02/25 19:43:58  kerman
RCS Log:change the unit system from BS to SI as it's necessary for decentred square
RCS Log:
RCS Log:Revision 2.2.4.5  2003/02/17 15:21:23  kerman
RCS Log:flip the z values in the fg structure
RCS Log:
RCS Log:Revision 2.2.4.4  2003/02/16 21:25:00  kerman
RCS Log:changing unit system from BS to CGS if necessary.:
RCS Log:
RCS Log:Revision 2.2.4.3  2003/01/22 16:52:14  rcatwood
RCS Log:Changed the sort order to conform with xuehua and weishan input routines.
RCS Log:
RCS Log:Revision 2.2.4.2  2003/01/17 16:09:33  rcatwood
RCS Log:Before changing all CA_FLOAT to CA_CA_FLOAT
RCS Log:
RCS Log:Revision 2.2.4.1  2003/01/15 20:05:41  rcatwood
RCS Log:*** empty log message ***
RCS Log:
RCS Log:Revision 2.2.2.2  2003/01/09 16:26:29  rcatwood
RCS Log:Sorted out several memory prblems.
RCS Log:Modified to handle allvac's data as well as fluent data
RCS Log:
RCS Log:Revision 2.2.2.1  2003/01/08 17:38:06  rcatwood
RCS Log:merge robert and ahmad read + sort + list programs
RCS Log:
RCS Log:Revision 2.1.2.1  2003/01/08 16:00:34  rcatwood
RCS Log:Ahmad version with sorting and input list of files to process.
RCS Log:
RCS Log:Revision 2.1  2002/10/17 16:56:01  rcatwood
RCS Log:*** empty log message ***
RCS Log:
RCS Log:Revision 1.4  2002/09/06 16:10:55  rcatwood
RCS Log:Changed line structure to double-dereferencet to improve reallocation efficiency
RCS Log:
RCS Log:Revision 1.3  2002/09/06 12:52:36  rcatwood
RCS Log:Cured all memory allocation problems .. I hope.
RCS Log:
RCS Log:Revision 1.2  2002/09/04 18:40:06  rcatwood
RCS Log:included FGrid_str structure from ca code , added routine to find number of nodes from input file
RCS Log:
RCS Log:Revision 1.1  2002/09/04 14:58:33  rcatwood
RCS Log:First working version -- reads and writes CSV files (no conversion)
RCS Log:
*/
