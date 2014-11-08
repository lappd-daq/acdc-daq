#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <math.h>

#include "SuMo.h"
//#include "H5Cpp.h"
#include "hdf5.h"

#define RANK          1
#define LENGTH        256
#define DATASETNAME   "PSEC4"

int SuMo::log_data_hd5(const char* log_filename, unsigned int NUM_READS, 
		       int trig_mode, int acq_rate, int boards){

  bool convert_to_voltage = false;
  int check_event, psec_cnt = 0;
  int sample;
  char log_data_filename[512];

  load_ped();
  sprintf(log_data_filename, "%s.hdf5", log_filename);
	
  /* First structure  and dataset*/  
  typedef struct s1_t {
    int pinfo;
    int pdat[AC_CHANNELS];
  } s1_t;
  
  /**HDF5 setup**/
  s1_t       s1[LENGTH];
  hid_t      s1_tid;     /* File datatype identifier */
  
  hsize_t   offset[1]={0};
  hsize_t    count[1]={LENGTH};
  size_t size;

  hid_t      file, dataset, memspace, space, pdat_type; /* Handles */
  herr_t     status;
  hsize_t    dim[1] = {LENGTH};   /* Dataspace dimensions */
  hsize_t    maxdim[1] = {H5S_UNLIMITED};   /* Max dataspace dimensions */
  hid_t      cparms;
  hsize_t    chkdim[1]={2};
  hsize_t    newsize[1]={LENGTH};
   
  /* Create the data space  */
  space = H5Screate_simple (RANK, dim, maxdim);

  file = H5Fcreate(log_data_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  printf ("H5Fcreate: %i\n", file);

  cparms = H5Pcreate (H5P_DATASET_CREATE);
  printf ("H5Pcreate: %i\n", cparms);
  status = H5Pset_chunk ( cparms, RANK, chkdim);
  printf ("H5Pset_chunk: %i\n", status);
    
  pdat_type = H5Tcopy (H5T_C_S1);
  size = AC_CHANNELS;
  status = H5Tset_size (pdat_type, size);
  printf ("H5Pset_size: %i\n", status);

  /* Create the memory data type */
  s1_tid = H5Tcreate (H5T_COMPOUND, sizeof(s1_t)); 
  H5Tinsert(s1_tid, "info", HOFFSET(s1_t, pinfo), H5T_NATIVE_INT);
  H5Tinsert(s1_tid, "data", HOFFSET(s1_t, pdat), pdat_type);

  /* Create the dataset  */
  dataset = H5Dcreate1 (file, DATASETNAME, s1_tid, space, cparms);
  printf ("H5Dcreate: %i\n", dataset);
   
  /* Create memory space for slab writes */
  memspace = H5Dget_space (dataset);
  printf ("H5Dget_space: %i\n", memspace);
  
  for(int k=0;k<NUM_READS; k++){
    reset_self_trigger();
    usleep(acq_rate);

    if(!trig_mode) software_trigger((unsigned int)15);
    
    usleep(1000);

    for(int targetAC = 0; targetAC < 4; targetAC++){
      if(targetAC == 0 && trig_mode == 1){
	manage_cc_fifo(1);
      }
      
      if(DC_ACTIVE[targetAC] == false){
	continue;
      }
      
      /* chip count */
      psec_cnt = 0;
      
      /* try readout: */
      if( read_AC(true, 1, targetAC ) == -1) 
	break; // go back to NUM_READS loop if no data to read
      
      /* if successful: */
      else{    
	/* get data */
	get_AC_info(false);

	s1[0].pinfo = unwrap(0);
	s1[1].pinfo = unwrap(1);
	s1[2].pinfo = unwrap(2); 
	s1[3].pinfo = unwrap(3); 
	s1[4].pinfo = unwrap(4); 
	s1[5].pinfo = k;
	
	check_event = 0;

	for(int i = 0; i < AC_CHANNELS; i++){
	  if(i>0 && i % 6 == 0) psec_cnt ++;

	  for(int j = 0; j < 256; j++){
	    sample = (int) AC_RAW_DATA[psec_cnt][i%6*256+j];
	    //sample -= (float) PED_DATA[targetAC][i][j];
	    s1[j].pdat[i] = sample;
	  }
	}  

      if((k+1) % 10 == 0)
	printf("Readout: %d of %d on board %d\n", k+1, NUM_READS, targetAC);
 
      /* more HDF5 handling */
      offset[0] = newsize[0];  
      newsize[0]= newsize[0]+LENGTH;
      
      status = H5Dextend (dataset, newsize);
      printf ("H5Dextend: %i\n", status);
       
      space = H5Dget_space (dataset);
      printf ("H5Dget_space: %i\n", space);

      status = H5Sselect_hyperslab (space, H5S_SELECT_SET, offset, NULL, count, NULL);
      printf ("H5Sselect_hyperslab: %i\n", status);

      /* Write data to the dataset */
      status = H5Dwrite(dataset, s1_tid, memspace, space, H5P_DEFAULT, s1);
      printf ("H5Dwrite: %i\n", status);

      /* Be sure to close the dataspace IN the loop to avoid using up lots of memory */
      status=H5Sclose(space);
      printf ("H5Sclose: %i\n", status);  
      
      } // ends else;
    
    } // ends looping over plugged in boards

    
  } // ends NUM_READS

  printf("...Finished Data Run...\n");
  printf("Data saved in file: %s\n", log_data_filename);
      
  /* Release resources */
  status= H5Pclose (cparms);
  //printf ("H5Pclose: %i\n", status);
  status= H5Tclose(s1_tid);
  // printf ("H5Tclose: %i\n", status);
  status=H5Sclose(memspace);
  //printf ("H5Sclose: %i\n", status);
  status=H5Dclose(dataset);
  //printf ("H5Dclose: %i\n", status);
  status=H5Fclose(file);
  // printf ("H5Fclose: %i\n", status);  
  
  return 0;
}
