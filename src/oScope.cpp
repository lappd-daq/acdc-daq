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
#include <math.h>
#include "SuMo.h"
#include "ScopePipe.h"

/* specific to file */
const int NUM_ARGS =      4;
const char* filename =     "oScope";
const char* description =  "pipe data to gnuplot window.";

static int scopeRefresh = 100000;
const  int SCOPE_AUTOSCALE = 200;

using namespace std;

int main(int argc, char* argv[]){
 
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1; 
  }
  else if(argc < NUM_ARGS  || argc > NUM_ARGS+1){
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
    int plotRange[]= {0, AC_CHANNELS};
    
    if(argc == NUM_ARGS+1){
      plotRange[0] = atoi(argv[4]);
      if(plotRange[0] > 25) plotRange[0] = 25;
      plotRange[1] = plotRange[0]+5;
    }    
      
    if(Sumo.check_active_boards(num_checks))
      return 1;
    
    int mode = Sumo.check_readout_mode();

    Sumo.oscilloscope(trigMode, numFrames, acdcNum, plotRange);
 
    return 0;
  }
} 

int SuMo::oscilloscope( int trig_mode, int numFrames, int AC_adr, int range[2] ){

  int device = 0;   //default is master board
  bool scopeBoard[numFrontBoards];
  for(int i=0; i<numFrontBoards; i++) scopeBoard[i] = false;
  scopeBoard[AC_adr] = true;

  if(AC_adr >= boardsPerCC && mode != USB2x){
    cout << "error, no USB slave device in readout configuration" << endl;
    return -2;
  }
  else if(AC_adr >= boardsPerCC && mode == USB2x){
    cout << "scoping slave device " << endl;
    device = 1; //slave device
  }

  int count = 0, psec_cnt = 0;
  int pdat[AC_CHANNELS][psecSampleCells];
  int max_pdat;
  int asic_baseline[psecSampleCells];
  int frameCount = 0;

  int sample;
  ScopePipe myPipe;
  string sendWord;

  if(myPipe.init()) return 1;
  
  //myPipe.send_cmd("set zrange [-1000:1000]");
  myPipe.send_cmd("set grid x z back");
  myPipe.send_cmd("set xyplane 0");
  myPipe.send_cmd("set view 14, 100, 1, 1.5");
  myPipe.send_cmd("set yrange [0:256]");
  
  myPipe.send_cmd("set xlabel \'channel\' ");
  myPipe.send_cmd("set ylabel \'sample no.\' ");
  //  myPipe.send_cmd("set zlabel \'counts\' ");

  if(DC_ACTIVE[AC_adr] == false){
    printf("no AC detected at specified address. cannot perform oscilloscope function!\n");
    return 1;
  }
  
  load_ped();
    
  unsigned int bit_address = pow(2, AC_adr % boardsPerCC);

  while(frameCount < numFrames){
    max_pdat = SCOPE_AUTOSCALE-1;
    psec_cnt = 0;
    if(device==1) manage_cc_fifo_slaveDevice(1);
    manage_cc_fifo(1);
    
    /* handle trigger */
    if(trig_mode){
      if(device==1) reset_self_trigger(15, 1);
      reset_self_trigger(15, 0);

      if(device==1) set_usb_read_mode_slaveDevice(7);
      set_usb_read_mode(7);
     
      usleep(scopeRefresh);

      if(device==1) set_usb_read_mode_slaveDevice(0);
      set_usb_read_mode(0);
    }
    /* otherwise, send trigger over software */
    else if(!trig_mode){ 
      if(device==1) set_usb_read_mode_slaveDevice(16), software_trigger_slaveDevice(1 << AC_adr-boardsPerCC);
      else          set_usb_read_mode(16), software_trigger(1 << AC_adr);
    }
    
    /* pull data from central card */
    read_AC(1, scopeBoard, false); 
    
    get_AC_info(false, AC_adr);
    
    for(int i = 0; i < AC_CHANNELS; i++){
      if(i>0 && i % 6 == 0) psec_cnt ++;
      
        for(int j = 0; j < psecSampleCells; j++){	
	sample  = adcDat[AC_adr]->AC_RAW_DATA[psec_cnt][i%6*256+j];
	sample -= PED_DATA[AC_adr][i][j];
	
	pdat[i][j] = sample;
	if(fabs(pdat[i][j]) >= SCOPE_AUTOSCALE) max_pdat = fabs(pdat[i][j]);
      }
    }

    /* scale z-axis */
    if(max_pdat < SCOPE_AUTOSCALE) myPipe.send_cmd("set zrange [-200:200]");
    else                           myPipe.send_cmd("set auto z");
    
    int baseline[psecSampleCells];
    
    unwrap_baseline(baseline, 2);   // firmware wraparound marked on ASIC #2 
    for (int j = 0; j < psecSampleCells; j++){
      asic_baseline[j] = baseline[j];
    }
     
    for(int i=range[0]; i < range[1]; i++){	  
      if(i==range[0]) myPipe.send_cmd("splot \'-\' using 1:2:3 notitle with linespoints, \\");
      else if(i==range[1]-1) myPipe.send_cmd("\'-\' using 1:2:3 notitle with linespoints");
      else
	myPipe.send_cmd("\'-\' using 1:2:3 notitle with linespoints, \\");
    }
	
    for(int i=range[0]; i < range[1]; i++){	  
      for(int j=0; j < psecSampleCells; j++){
	char sendWord[100];
	sprintf(sendWord, "%d %d %d", i, j, pdat[i][asic_baseline[j]]);
	myPipe.send_cmd(sendWord);	        
      }
      myPipe.send_cmd("eof");	        
    }

    frameCount++;
  }
  /*hold window*/
  cout << "press enter to quit"; 
  cin.ignore();
  return 0; 
}
