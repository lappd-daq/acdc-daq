/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// oScope.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"
#include "ScopePipe.h"

/* specific to file */
const int NUM_ARGS =      4;
const char* filename =     "oScope";
const char* description =  "pipe data to gnuplot window.";

const int scopeRefresh = 800000;

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
  /* function defined below */
  else{
    SuMo Sumo;

    int num_checks = 5;
    int acdcNum    = atoi(argv[1]);
    int numFrames  = atoi(argv[2]);
    int trigMode   = atoi(argv[3]);
    
    if(Sumo.check_active_boards(num_checks))
      return 1;
    
    Sumo.oscilloscope(trigMode, numFrames, acdcNum);
 
    return 0;
  }
} 

int SuMo::oscilloscope( int trig_mode, int numFrames, int AC_adr){
  bool convert_to_voltage = false;
  int check_event, count = 0, psec_cnt = 0;
  float pdat[AC_CHANNELS][psecSampleCells];
  int asic_baseline[psecSampleCells];
  int targetAC = 0;
  int frameCount = 0;

  float sample;
  ScopePipe myPipe;
  string sendWord;

  if(myPipe.init()) return 1;

  //myPipe.send_cmd("set zrange [-1000:1000]");

  if(DC_ACTIVE[AC_adr] == false){
    printf("no AC detected at specified address. cannot perform oscilloscope function!\n");
    return 1;
  }
  
  load_ped();

  while(frameCount < numFrames){
    psec_cnt = 0;
    manage_cc_fifo(1);
    if(trig_mode) set_usb_read_mode(7);
    if(!trig_mode){ 
      set_usb_read_mode(AC_adr);
      software_trigger((unsigned int)15);
    }
    
    read_AC(true, 1, AC_adr); 
    check_event = 0;
    get_AC_info(false, AC_adr);
    
    for(int i = 0; i < AC_CHANNELS; i++){
      if(i>0 && i % 6 == 0) psec_cnt ++;
      
      for(int j = 0; j < psecSampleCells; j++){
	if(convert_to_voltage){
	  //sample =  LUT[(int)AC_RAW_DATA[psec_cnt][i%6*256+j]][i]*1000;
	  //sample -= LUT[(int)PED_DATA[i][j]][i]*1000;
	}
	else{
	  sample = (float) acdcData[AC_adr].AC_RAW_DATA[psec_cnt][i%6*256+j];
	  sample -= (float) PED_DATA[AC_adr][i][j];
	}
	pdat[i][j] = sample;
      }
    }
    
    int baseline[psecSampleCells];
    
    unwrap_baseline(baseline, 2);   // firmware wraparound marked on ASIC #2 
    for (int j = 0; j < psecSampleCells; j++){
      asic_baseline[j] = baseline[j];
      }
     
    for(int i=0; i < AC_CHANNELS; i++){	  
      if(i==0) myPipe.send_cmd("splot \'-\' using 1:2:3 notitle with lines, \\");
      else if(i==AC_CHANNELS-1) myPipe.send_cmd("\'-\' using 1:2:3 notitle with lines");
      else
	myPipe.send_cmd("\'-\' using 1:2:3 notitle with lines, \\");
    }
	
    for(int i=0; i < AC_CHANNELS; i++){	  
      for(int j=0; j < psecSampleCells; j++){
	char sendWord[100];
	sprintf(sendWord, "%d %d %f", i, j, pdat[i][asic_baseline[j]]);
	myPipe.send_cmd(sendWord);	        
      }
      myPipe.send_cmd("eof");	        
    }

    usleep(100);
    frameCount++;
    //manage_cc_fifo(1);
    //if(trig_mode) set_usb_read_mode(7);
    manage_cc_fifo(1);
  }
  /*hold window*/
  int temp;
  cout << "press enter to quit"; 
  cin.ignore();
  return 0; 
}
