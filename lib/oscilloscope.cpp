#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <signal.h>
#include "oscilloscope.h"
#include "Packet.h"
#include "Timer.h"

static int scopeRefresh = 30000;
const  int SCOPE_AUTOSCALE = 200;
const  int SCOPE_TIMEOUT   = 40;

using namespace std;

/* If numFrames is a negative number, set the oscope up in "infinity" mode were it will run until manually stopped */
int oscilloscope(SuMo& Sumo, int trig_mode, int numFrames, int boardID, int range[2] ){
  
  Timer timer = Timer();
  int device = 0;   //default is master board
  bool scopeBoard[numFrontBoards];
  int pdat[AC_CHANNELS][psecSampleCells];
  int max_pdat;

  ScopePipe myPipe;

  device = setup_scope(Sumo, boardID);
  if(device < 0) return -1;
  if(setup_scope(myPipe, boardID) == 1) return -2;

  Sumo.load_ped();
  timer.start();
  float t;

  for(int i=0; i<numFrontBoards; i++) scopeBoard[i] = false;
  scopeBoard[boardID] = true;
  if (numFrames < 0) {
    while(t < SCOPE_TIMEOUT) {
      t = timer.stop();
      if (prime_scope(Sumo, trig_mode, device, boardID, scopeRefresh) == 0) continue;
      max_pdat = log_from_scope(Sumo, boardID, pdat, scopeBoard);

      if (max_pdat < SCOPE_AUTOSCALE) myPipe.send_cmd("set zrange [-200:200]");
      else myPipe.send_cmd("set auto z");

      plot_scope(myPipe, pdat, range);
    }
  } else {
    for (int frameCount = 0; frameCount < numFrames && t < SCOPE_TIMEOUT; frameCount++) {
      t = timer.stop();
      if (prime_scope(Sumo, trig_mode, device, boardID, scopeRefresh) == 0) continue;
      max_pdat = log_from_scope(Sumo, boardID, pdat, scopeBoard);

      if (max_pdat < SCOPE_AUTOSCALE) myPipe.send_cmd("set zrange [-200:200]");
      else myPipe.send_cmd("set auto z");

      plot_scope(myPipe, pdat, range);
    }
  }
  Sumo.cleanup();
  cout << "press enter to quit";
  cin.ignore();
  return 0;
}


int setup_scope(SuMo& Sumo, int boardID){
  if(Sumo.DC_ACTIVE[boardID] == false){
    printf("no AC detected at specified address. cannot perform oscilloscope function!\n");
    return -1;
  }

  int device = 0;

  if(boardID >= boardsPerCC && Sumo.check_readout_mode()!=1){
    cout << "error, no USB slave device in readout configuration" << endl;
    return -2;
  }
  else if(boardID >= boardsPerCC && Sumo.check_readout_mode()==1){
    cout << "scoping slave device " << endl;
    device = 1; //slave device
  }

  if(Sumo.check_readout_mode()==1)  Sumo.set_usb_read_mode_slaveDevice(0);                
  Sumo.set_usb_read_mode(0); 

  return device;

}
int setup_scope(ScopePipe& myPipe, int boardID){

  if(myPipe.init()) return 1;
  //myPipe.send_cmd("set zrange [-1000:1000]");
  myPipe.send_cmd("set term wxt size 610, 480");
  myPipe.send_cmd("set grid x z back");
  myPipe.send_cmd("set xyplane 0");
  myPipe.send_cmd("set view 57, 105, 1, 1.5");
  myPipe.send_cmd("set yrange [0:256]");
  
  myPipe.send_cmd("set xlabel \'channel\' ");
  myPipe.send_cmd("set ylabel \'sample no.\' ");
  //  myPipe.send_cmd("set zlabel \'counts\' ");
  return 0;
}

int prime_scope(SuMo& Sumo, int trig_mode, int device, int boardID, int scopeRefresh){
  if(Sumo.check_readout_mode()==1) Sumo.manage_cc_fifo_slaveDevice(1);
  Sumo.manage_cc_fifo(1);

    if(trig_mode == 1){
      if(Sumo.check_readout_mode()==1) Sumo.reset_self_trigger(15, 1);
      Sumo.reset_self_trigger(15, 0);
      Sumo.prep_sync();
      if(Sumo.check_readout_mode()==1) Sumo.system_slave_card_trig_valid(true);
      Sumo.system_card_trig_valid(true);
      Sumo.make_sync();
      
      Sumo.sys_wait(scopeRefresh);
      
      Sumo.system_card_trig_valid(false);
      if(Sumo.check_readout_mode()==1) Sumo.system_slave_card_trig_valid(false); 
    }
    // otherwise, send trigger over software 
    else if(trig_mode == 0){ 
      if(device==1) Sumo.software_trigger_slaveDevice(1 << boardID-boardsPerCC);
      else          Sumo.software_trigger(1 << boardID);

      Sumo.sys_wait(1000);
    }
    
    //int evts = Sumo.read_CC(false, false, 100);
    int evts = Sumo.read_CC(false, false, 0);
    cout << evts << ":";
    for(int j=0; j<numFrontBoards; j++){
      cout << Sumo.EVENT_FLAG[j];
    } 
    cout << " \r";
    cout.flush();

    if( Sumo.EVENT_FLAG[boardID] == 0 ) return 0;

    return evts;
}

int log_from_scope(SuMo& Sumo,  int boardID, int pdat[AC_CHANNELS][psecSampleCells],bool* scopeBoard){
  int psec_cnt     = 0;
  int sample       = 0;
  int maxADC_count = 0;

  Sumo.read_AC(1, scopeBoard, false);
  Sumo.set_usb_read_mode(0);
  if(Sumo.check_readout_mode()==1) Sumo.set_usb_read_mode_slaveDevice(0);
  Sumo.get_AC_info(false, boardID);
      
  for(int i = 0; i < AC_CHANNELS; i++){
      if(i>0 && i % 6 == 0) psec_cnt ++;
      
        for(int j = 0; j < psecSampleCells; j++){	
	sample  = Sumo.adcDat[boardID]->AC_RAW_DATA[psec_cnt][i%6*256+j];
	sample -= Sumo.PED_DATA[boardID][i][j];
	//	cout << " " << sample;
	pdat[i][j] = sample;
	if(fabs(pdat[i][j]) >= SCOPE_AUTOSCALE) maxADC_count = fabs(pdat[i][j]);
      }
    }

  return maxADC_count;
}

int plot_scope(ScopePipe& myPipe, int pdat[AC_CHANNELS][psecSampleCells], int* range){
  string sendWord;
  int baseline[psecSampleCells];
  int asic_baseline[psecSampleCells];

  for (int j = 0; j < psecSampleCells; j++){
    asic_baseline[j] = j;
  }
     
  for(int i=range[0]; i < range[1]; i++){	  
    if(i==range[0])        myPipe.send_cmd("splot \'-\' using 1:2:3 notitle with linespoints, \\");
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
  return 0;
}
