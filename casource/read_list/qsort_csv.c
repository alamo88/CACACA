/*RCS Id:$Id: qsort_csv.c 887 2006-03-01 18:21:01Z rcatwood $*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "machine.h"
#include "constants.h"
#include "csv.h"
#include "fidap.h"
#include "read_fg_list.h"
#include "init.h"

int main_col=0;
int second_col=0;

int compare2(const void * v_row, const void * v_row_next) 
{
CA_FLOAT ** row;
CA_FLOAT ** row_next;
int Result=0;
row=(CA_FLOAT **) (v_row);
row_next=(CA_FLOAT **) (v_row_next);


if((*row)[main_col] < (*row_next)[main_col]){
   Result = 1;
} else if((*row)[main_col] > (*row_next)[main_col]){
   Result = -1;
}else{
	if((*row)[second_col]>(*row_next)[second_col]){
		Result=+1;
	}
	if((*row)[second_col]<(*row_next)[second_col]){
		Result=-1;
	}
}
return(Result);
}


void qsort_csv(CsvFloat * the_data, int z_col, int r_col)
{
  main_col=z_col;
  second_col=r_col;
     qsort(the_data->data, the_data->line_count, sizeof(CA_FLOAT *), compare2);
} 

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
RCS Log:Revision 9.2  2003/12/10 17:36:23  kerman
RCS Log:added unit conversions & fixed for counting number of fields from the header
RCS Log:
RCS Log:Revision 1.1.2.3  2003/02/25 19:43:58  kerman
RCS Log:change the unit system from BS to SI as it's necessary for decentred square
RCS Log:
RCS Log:Revision 1.1.6.2  2003/01/22 16:52:15  rcatwood
RCS Log:Changed the sort order to conform with xuehua and weishan input routines.
RCS Log:
RCS Log:Revision 1.1.6.1  2003/01/15 20:05:41  rcatwood
RCS Log:*** empty log message ***
RCS Log:
RCS Log:Revision 1.1.4.1  2003/01/08 17:38:57  rcatwood
RCS Log:adding new files neede for readin gthe list of files
RCS Log:
RCS Log:Revision 1.1.2.1  2003/01/08 16:50:15  rcatwood
RCS Log:added files for list processing
RCS Log:
RCS Log:Revision 2.1  2002/10/17 16:56:01  rcatwood
RCS Log:*** empty log message ***
RCS Log:
RCS Log:Revision 1.4  2002/09/06 16:10:55  rcatwood
RCS Log:Changed line structure to double-dereferencet to improve reallocation efficiency
RCS Log:
RCS Log:Revision 1.3  2002/09/05 18:05:44  rcatwood
RCS Log:Included convert, read and write fg binary, and tested. It seems to work.
RCS Log:
RCS Log:Revision 1.2  2002/09/04 18:40:06  rcatwood
RCS Log:included FGrid_str structure from ca code , added routine to find number of nodes from input file
RCS Log:
RCS Log:Revision 1.1  2002/09/04 14:58:33  rcatwood
RCS Log:First working version -- reads and writes CSV files (no conversion)
RCS Log:
*/
