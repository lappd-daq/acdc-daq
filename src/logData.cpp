#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <sys/stat.h>

#include "SuMo.h"

/* specific to file */
const int NUM_ARGS = 4;
const char* filename = "logData";
const char* description = "log data from DAQ";
using namespace std;

bool fileExists(const std::string& filename);

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
  float sample;
  char logDataFilename[300];
  int asic_baseline[psecSampleCells];
  int LIMIT_READOUT_RATE = 10000;

  /* handle filename */
  sprintf(logDataFilename, "%s.txt", log_filename);
  string temp;
  while(fileExists(logDataFilename)){
    cout << "file already exists, try new filename: (or enter to overwrite): ";
    getline(cin, temp);
    if(temp.empty()) break;
    sprintf(logDataFilename, "%s.txt", temp.c_str());
  }
  ofstream ofs;
  ofs.open(logDataFilename, ios::trunc);

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
    /*set read mode to NULL */
    set_usb_read_mode(16);
    /*reset last event on firmware */
    manage_cc_fifo(1);
    /*send trigger over software if not looking externally */
    if(trig_mode){ 
      set_usb_read_mode(7);
      usleep(acq_rate+LIMIT_READOUT_RATE);
    }
    else{
      software_trigger((unsigned int)15);
      usleep(LIMIT_READOUT_RATE); /* somewhat arbitrary hard-coded rate limitation */
    }

    /* show event number at terminal */
    if((k+1) % 2 == 0){
      cout << "Readout:  " << k+1 << " of " << NUM_READS << "      \r";
      cout.flush();
    }
    /*Do bulk read on all front-end cards */
    int numBoards = read_AC(1);
    if(numBoards == 0) continue; 

    /* form data for filesave */
    for(int targetAC = 0; targetAC < 4; targetAC++){ 
      if(BOARDS_READOUT[targetAC] && numBoards > 0){
	psec_cnt = 0;
	/*assign meta data */
	get_AC_info(false, targetAC);
	acdcData[targetAC].Data[AC_CHANNELS][0] = k;
	acdcData[targetAC].Data[AC_CHANNELS][1] = CC_EVENT_NO;
	acdcData[targetAC].Data[AC_CHANNELS][2] = CC_BIN_COUNT; 
	acdcData[targetAC].Data[AC_CHANNELS][3] = WRAP_CONSTANT; 
	acdcData[targetAC].Data[AC_CHANNELS][4] = acdcData[targetAC].RO_CNT[0]; 
	acdcData[targetAC].Data[AC_CHANNELS][5] = acdcData[targetAC].RO_CNT[1];
	acdcData[targetAC].Data[AC_CHANNELS][6] = acdcData[targetAC].RO_CNT[2];
	acdcData[targetAC].Data[AC_CHANNELS][7] = acdcData[targetAC].RO_CNT[3];
	acdcData[targetAC].Data[AC_CHANNELS][8] = acdcData[targetAC].RO_CNT[4];
	acdcData[targetAC].Data[AC_CHANNELS][9] = acdcData[targetAC].RO_CNT[5];
	acdcData[targetAC].Data[AC_CHANNELS][10] = acdcData[targetAC].VBIAS[0]; 
	acdcData[targetAC].Data[AC_CHANNELS][11] = acdcData[targetAC].VBIAS[1];
	acdcData[targetAC].Data[AC_CHANNELS][12] = acdcData[targetAC].VBIAS[2];
	acdcData[targetAC].Data[AC_CHANNELS][13] = acdcData[targetAC].VBIAS[3];
	acdcData[targetAC].Data[AC_CHANNELS][14] = acdcData[targetAC].VBIAS[4];
	acdcData[targetAC].Data[AC_CHANNELS][15] = acdcData[targetAC].VBIAS[5];
	check_event = 0;
	//printf("#%d\n", k);
	for(int i = 0; i < AC_CHANNELS; i++){
	  if(i>0 && i % 6 == 0) psec_cnt ++;

	  for(int j = 0; j < psecSampleCells; j++){
	    if(convert_to_voltage){
	      //sample =  LUT[(int)AC_RAW_DATA[psec_cnt][i%6*256+j]][i]*1000;
	      //sample -= LUT[(int)PED_DATA[targetAC][i][j]][i]*1000;
	    }
	    else{
	      sample = (float) acdcData[targetAC].AC_RAW_DATA[psec_cnt][i%6*256+j];
	      sample -= (float) PED_DATA[targetAC][i][j];
	    }
	    acdcData[targetAC].Data[i][j] = sample;
	  }
	}  
	/* wraparound_correction, if desired: */
	int baseline[256];
	unwrap_baseline(baseline, 2); 
	for (int j = 0; j < psecSampleCells; j++){
	  asic_baseline[j] = baseline[j];
	}
      }
    }
	
    for(int i=0; i < psecSampleCells; i++){

      ofs << i << " " << asic_baseline[i] << " ";
      for(int board=0; board<numFrontBoards; board++){
	if(BOARDS_READOUT[board]){
	  for(int channel=0; channel < AC_CHANNELS+1; channel++){	    
	    ofs << std::dec << acdcData[board].Data[channel][i] << " ";
	  }
	}
      }
      ofs <<endl;
    } 
  }
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << "  .....Finished Data Run......      \r";
  cout << endl;

  manage_cc_fifo(1);
  //set_usb_read_mode(7);
  ofs <<endl<<endl;
  ofs.close();

  set_usb_read_mode(16);  //turn off trigger, if on
  dump_data();
  
  cout << "Data saved in file: " << logDataFilename << endl;
  return 0;
}

bool fileExists(const string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}
