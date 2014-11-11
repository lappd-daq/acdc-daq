#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS = 4;
const char* filename = "logData";
const char* description = "log data from DAQ";
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

    command.log_data(log_data_filename, num_events, trig_mode, acq_rate);
    
    return 0;
  }
}

int SuMo::log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate){

  bool convert_to_voltage;
  int check_event, count = 0, psec_cnt = 0;
  float pdat[AC_CHANNELS][256];
  float sample;
  char log_data_filename[512];
  int asic_baseline[256];
  FILE* fdatout[4]; 
  
  int pinfo[256];
  
  for (int i = 0; i < 256; i++){
      pinfo[i] = 0;
    }

  for(int targetAC = 0; targetAC < 4; targetAC++){
    if(DC_ACTIVE[targetAC] == true){
      printf("logging data for board %d\n", targetAC);
      sprintf(log_data_filename, "%s_board%d.txt", log_filename, targetAC);
      fdatout[targetAC] = fopen(log_data_filename, "w");
    }
  } 
  
  convert_to_voltage = false;
  /*if(load_lut() != 0) convert_to_voltage = false;
   *else
   *  convert_to_voltage = output_mode;
   */
  //setup
  dump_data();
  if(trig_mode){
    set_usb_read_mode(24);
  }
  else{
    set_usb_read_mode(16);
    dump_data();
  }

  load_ped();

  for(int k=0;k<NUM_READS; k++){

    //reset_self_trigger();
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
      //manage_cc_fifo(1);
      //}
      if(DC_ACTIVE[targetAC] == false){		  
	continue;
      }
      psec_cnt = 0;
      //try readout:
      read_AC(true, 1, targetAC);
	      //if(read_AC(true, 1, targetAC) == -1) {
	      //	printf("no data on board %d\n", targetAC);
	      	      //	break; // go back to NUM_READS loop if no data to read
	      // }
      //if successful:
      if(1){
      //else{
	get_AC_info(false);
	pinfo[0] = k;
	pinfo[1] = CC_EVENT_NO;
	pinfo[2] = CC_BIN_COUNT; 
	pinfo[3] = WRAP_CONSTANT; 
	pinfo[4] = RO_CNT[0]; 
	pinfo[5] = RO_CNT[1];
	pinfo[6] = RO_CNT[2];
	pinfo[7] = RO_CNT[3];
	pinfo[8] = RO_CNT[4];
	pinfo[9] = RO_CNT[5];
	pinfo[10] = VBIAS[0]; 
	pinfo[11] = VBIAS[1];
	pinfo[12] = VBIAS[2];
	pinfo[13] = VBIAS[3];
	pinfo[14] = VBIAS[4];
	pinfo[15] = VBIAS[5];
	check_event = 0;
	//printf("#%d\n", k);
	for(int i = 0; i < AC_CHANNELS; i++){
	  if(i>0 && i % 6 == 0) psec_cnt ++;

	  for(int j = 0; j < 256; j++){
	    if(convert_to_voltage){
	      //sample =  LUT[(int)AC_RAW_DATA[psec_cnt][i%6*256+j]][i]*1000;
	      //sample -= LUT[(int)PED_DATA[targetAC][i][j]][i]*1000;
	    }
	    else{
	      sample = (float) AC_RAW_DATA[psec_cnt][i%6*256+j];
	      sample -= (float) PED_DATA[targetAC][i][j];
	    }
	    pdat[i][j] = sample;
	  }
	}  
	//wraparound_correction, if desired:
	int baseline[256];
	unwrap_baseline(baseline, 2); 
	for (int j = 0; j < 256; j++){
	  asic_baseline[j] = baseline[j];
	}
	for(int i=0; i < 256; i++){
	  /*
	  fprintf(fdatout[targetAC], 
		  "%4.1d\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%d\t\n",    
		  i,pdat[0][i],pdat[1][i],pdat[2][i],
		  pdat[3][i],pdat[4][i],pdat[5][i],
		  pdat[6][i],pdat[7][i],pdat[8][i],
		  pdat[9][i],pdat[10][i],pdat[11][i],
		  pdat[12][i],pdat[13][i],pdat[14][i],
		  pdat[15][i],pdat[16][i],pdat[17][i],
		  pdat[18][i],pdat[19][i],pdat[20][i],
		  pdat[21][i],pdat[22][i],pdat[23][i],
		  pdat[24][i],pdat[25][i],pdat[26][i],
		  pdat[27][i],pdat[28][i],pdat[29][i],
		  pinfo[i]);
	  */
	  fprintf(fdatout[targetAC], 
		  "%4.1d\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%d\t\n",    
		  i,pdat[0][asic_baseline[i]],pdat[1][asic_baseline[i]],pdat[2][asic_baseline[i]],
		  pdat[3][asic_baseline[i]],pdat[4][asic_baseline[i]],pdat[5][asic_baseline[i]],
		  pdat[6][asic_baseline[i]],pdat[7][asic_baseline[i]],pdat[8][asic_baseline[i]],
		  pdat[9][asic_baseline[i]],pdat[10][asic_baseline[i]],pdat[11][asic_baseline[i]],
		  pdat[12][asic_baseline[i]],pdat[13][asic_baseline[i]],pdat[14][asic_baseline[i]],
		  pdat[15][asic_baseline[i]],pdat[16][asic_baseline[i]],pdat[17][asic_baseline[i]],
		  pdat[18][asic_baseline[i]],pdat[19][asic_baseline[i]],pdat[20][asic_baseline[i]],
		  pdat[21][asic_baseline[i]],pdat[22][asic_baseline[i]],pdat[23][asic_baseline[i]],
		  pdat[24][asic_baseline[i]],pdat[25][asic_baseline[i]],pdat[26][asic_baseline[i]],
		  pdat[27][asic_baseline[i]],pdat[28][asic_baseline[i]],pdat[29][asic_baseline[i]],
		  pinfo[i]);
	  
	}
      }      
    }
  }
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << "...Finished Data Run...      \r";
  cout << endl;

  manage_cc_fifo(1);
  //set_usb_read_mode(7);
  
  for(int targetAC = 0; targetAC < 4; targetAC++){
    if(DC_ACTIVE[targetAC] == true){
      int file_close_retval = fclose(fdatout[targetAC]);
      //printf("%d,\n", file_close_retval);
    }
  }
  set_usb_read_mode(16);  //turn off trigger, if on
  dump_data();
  
  printf("Data saved in file: %s_boardX\n", log_filename);
  return 0;
}
