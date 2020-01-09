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
#include <signal.h>
#include "SuMo.h"
#include "oscilloscope.h"
#include "Timer.h"

/* specific to file */
const int NUM_ARGS =      4;
const char* filename =     "oScope";
const char* description =  "pipe data to gnuplot window.";

int interrupt_flag = 0;
using namespace std;

int main(int argc, char* argv[]){

  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1; 
  }
  else if(argc < NUM_ARGS  || argc > NUM_ARGS+1) {
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
    
    Sumo.check_readout_mode();

    oscilloscope(Sumo, trigMode, numFrames, acdcNum, plotRange);
 
    return 0;
  }
} 
