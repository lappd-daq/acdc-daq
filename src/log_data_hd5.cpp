#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <math.h>

#include "SuMo.h"
//#include "H5Cpp.h"
#include "hdf5.h"

#define RANK          2
#define DATASETNAME   "PSEC4_ACDC"
#define LENGTH        256

int SuMo::log_data_hd5(const char* log_filename, unsigned int NUM_READS, 
		       int trig_mode, int acq_rate, int boards){

  bool convert_to_voltage = false;
  int check_event, psec_cnt = 0;
  int sample;
  char log_data_filename[512];

  load_ped();
  sprintf(log_data_filename, "%s.hdf5", log_filename);  

  int pdat[256][AC_CHANNELS + 1]; // Row major declaration

  //typedef struct data_t {
  //  int pdata[256][AC_CHANNELS];
  //  int pinfo[10];
  //  int pevent;
  //} data_t;

  for(int i=0; i<LENGTH; i++){
    for(int j=0; j< (AC_CHANNELS + 1); j++){
	pdat[i][j] = 0;	
    }
  }
  
  //data_t saveData;
  hid_t file;
  hid_t dataspace, dataset;
  hid_t filespace;
  hid_t cparms;
  hsize_t dims[2] = { LENGTH, AC_CHANNELS + 1};
  hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
  hsize_t chunk_dims[2] = { 2, 5}; //arbitrary?
  hsize_t size[2];
  hsize_t offset[2];

  herr_t status;

  dataspace = H5Screate_simple(RANK, dims, maxdims); 
  file = H5Fcreate(log_data_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  cparms = H5Pcreate (H5P_DATASET_CREATE);
  status = H5Pset_chunk( cparms, RANK, chunk_dims);
  dataset = H5Dcreate1(file, DATASETNAME, H5T_NATIVE_INT, dataspace,
		      cparms);

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

	/* Extend dataset on each read */
	size[0] = dims[0] * (k + 1);
	size[1] = dims[1];
	status = H5Dextend (dataset, size);
	
	/* select a hyperslab */
	filespace = H5Dget_space (dataset);
	offset[0] = LENGTH * k;
	offset[1] = 0;
	status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL,
				     dims, NULL);  

	get_AC_info(false);

	// /*
	pdat[1][30] = unwrap(0);
	pdat[2][30] = unwrap(1);
	pdat[3][30] = unwrap(2); 
	pdat[4][30] = unwrap(3); 
	pdat[5][30] = unwrap(4); 
	pdat[0][30]  = k;
	// */

	/*
	saveData.pinfo[0] = unwrap(0);
	saveData.pinfo[1] = unwrap(1);
	saveData.pinfo[2] = unwrap(2);
	saveData.pinfo[3] = unwrap(3);
	saveData.pinfo[4] = unwrap(4);
	saveData.pevent = k;
	*/
	check_event = 0;

	for(int i = 0; i < AC_CHANNELS; i++){
	  if(i>0 && i % 6 == 0) psec_cnt ++;

	  for(int j = 0; j < 256; j++){
	    sample = (int) AC_RAW_DATA[psec_cnt][i%6*256+j];
	    sample -=  PED_DATA[targetAC][i][j];
	    pdat[j][i] = sample;
	    //saveData.pdata[j][i] = sample;
	  }
	}  

      if((k+1) % 10 == 0)
	printf("Readout: %d of %d on board %d\n", k+1, NUM_READS, targetAC);


      /* Write data to the dataset */
      status = H5Dwrite(dataset, H5T_NATIVE_INT, dataspace, filespace,
			H5P_DEFAULT, pdat);
      //status = H5Dwrite(dataset, H5T_NATIVE_INT, dataspace, filespace,
      //H5P_DEFAULT, saveData);
      
      //printf ("H5Dwrite: %i\n", status);
      
      } // ends else;
    
    } // ends looping over plugged in boards

    
  } // ends NUM_READS

  printf("...Finished Data Run...\n");
  printf("Data saved in file: %s\n", log_data_filename);
      
  /* Release resources */
  H5Dclose(dataset);
  H5Sclose(dataspace);
  H5Sclose(filespace);
  H5Fclose(file);

  return 0;
}
