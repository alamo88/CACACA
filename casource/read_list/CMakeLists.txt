#Cmake project file for uMat IC microstructure simulation software 
#By Robert Atwood
#$Id: CMakeLists.txt 1402 2008-11-20 15:36:41Z  $


#It is easiest to provide these functions in a library file
#when using CMake

#This creates libreadlist.a  or .so
#List the c files containing the library functions
add_definitions( -fPIC )
add_definitions( ${REG_LCDEFS} )

   if(UMAT_32)
   add_definitions(-m32)
   set(CMAKE_EXE_LINKER_FLAGS -m32)
   set(CMAKE_SHARED_LINKER_FLAGS -m32)
   endif(UMAT_32)

add_library(readlist 
   convert.c
   debin.c
   findvals.c
   freecsv.c
   init.c
   qsort_csv.c
   read_fg_list.c
   readwrite.c
   readwrite_data.c
   readwrite_fg.c
   unit_conv.c
)
