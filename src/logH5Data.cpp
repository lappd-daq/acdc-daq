#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "SuMo.h"

#include <cstdlib>
#include <math.h>

//#include "H5Cpp.h"
#include "hdf5.h"

#define RANK          2
#define DATASETNAME   "PSEC4_ACDC"
#define LENGTH        256

/* specific to file */
const int NUM_ARGS =  4;
const char* filename = "logH5Data";
const char* description = "log data from DAQ, save to hdf5 compressed format";
using namespace std;

int main(int argc, char* argv[]){
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1; 
  }
  else if(argc != NUM_ARGS){
    cout << "error: wrong number of arguments" << endl;
    return -1;
  }
  else{
    int num_checks = 5; 
    int num_events = 100;
    int acq_rate   = 10000;
    SuMo command;
    char log_data_filename[100];
    
    strcpy(log_data_filename, argv[1]);
    num_events = atoi(argv[2]);
    int trig_mode = atoi(argv[3]);
    
    command.set_usb_read_mode(16); 
    if(command.check_active_boards(num_checks))
      return 1;

    command.log_data_hd5(log_data_filename, num_events, trig_mode, acq_rate);
    
    return 0;
  }
}

int SuMo::log_data_hd5(const char* log_filename, unsigned int NUM_READS, 
		       int trig_mode, int acq_rate){

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
  dump_data();
  if(trig_mode){
    set_usb_read_mode(24);
  }
  else{
    set_usb_read_mode(16);
    dump_data();
  }
  
  for(int k=0;k<NUM_READS; k++){
    reset_self_trigger();
    manage_cc_fifo(1);

    if(trig_mode){ 
      set_usb_read_mode(7);
      usleep(acq_rate);
    }
    else{
      software_trigger((unsigned int)15);
    }

    if((k+1) % 2 == 0){
      cout << "Readout:  " << k+1 << " of " << NUM_READS << "      \r";
      cout.flush();
    }
    
    for(int targetAC = 0; targetAC < 4; targetAC++){
      //if(targetAC == 0 && trig_mode == 1){
      //	manage_cc_fifo(1);
      //}
      
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

	get_AC_info(false, targetAC);
	// /*
	pdat[0][30] = k;
	pdat[1][30] = CC_EVENT_NO;
	pdat[2][30] = CC_BIN_COUNT; 
	pdat[3][30] = WRAP_CONSTANT; 
	pdat[4][30] = acdcData[targetAC].RO_CNT[0]; 
	pdat[5][30] = acdcData[targetAC].RO_CNT[1];
	pdat[6][30] = acdcData[targetAC].RO_CNT[2];
	pdat[7][30] = acdcData[targetAC].RO_CNT[3];
	pdat[8][30] = acdcData[targetAC].RO_CNT[4];
	pdat[9][30] = acdcData[targetAC].RO_CNT[5];
	pdat[10][30] = acdcData[targetAC].VBIAS[0]; 
	pdat[11][30] = acdcData[targetAC].VBIAS[1];
	pdat[12][30] = acdcData[targetAC].VBIAS[2];
	pdat[13][30] = acdcData[targetAC].VBIAS[3];
	pdat[14][30] = acdcData[targetAC].VBIAS[4];
	pdat[15][30] = acdcData[targetAC].VBIAS[5];
 
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
	    sample = (int) acdcData[targetAC].AC_RAW_DATA[psec_cnt][i%6*256+j];
	    sample -=  PED_DATA[targetAC][i][j];
	    pdat[j][i] = sample;
	    //saveData.pdata[j][i] = sample;
	  }
	}  


      /* Write data to the dataset */
      status = H5Dwrite(dataset, H5T_NATIVE_INT, dataspace, filespace,
			H5P_DEFAULT, pdat);
      //status = H5Dwrite(dataset, H5T_NATIVE_INT, dataspace, filespace,
      //H5P_DEFAULT, saveData);
      
      //printf ("H5Dwrite: %i\n", status);
      
      } // ends else;
    
    } // ends looping over plugged in boards

    
  } // ends NUM_READS

  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << "...Finished Data Run...      \r";
  cout << endl;
  manage_cc_fifo(1);
  set_usb_read_mode(16);  //turn off trigger, if on
  dump_data();
  printf("Data saved in file: %s\n", log_filename);
      
  /* Release resources */
  H5Dclose(dataset);
  H5Sclose(dataspace);
  H5Sclose(filespace);
  H5Fclose(file);

  return 0;
}
