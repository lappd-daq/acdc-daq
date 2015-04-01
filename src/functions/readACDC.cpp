/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// readACDC.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      1;
const char* filename =     "readACDC";
const char* description =  "show front-end status";

using namespace std;

int main(int argc, char* argv[]){
 
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1; 
  }
  else if(argc > NUM_ARGS+1){
    cout << "error: wrong number of arguments" << endl;
    return -1;
  }
  else{
    SuMo Sumo;
    int num_checks = 3;  
    if(Sumo.check_active_boards(num_checks)) return 1;

  
    if(argc == NUM_ARGS+1 && (std::string(argv[1]) == "-sync" || std::string(argv[1]) == "-s")){
      cout << "**reading ACDC data in sync mode.." << endl << "**" << endl;
    
      Sumo.check_active_boards(true);

      Sumo.read_CC(true, true, 100,0);
    }
    else if(argc == NUM_ARGS+1 && (std::string(argv[1]) == "-pull" || std::string(argv[1]) == "-p")){
      cout << "**reading ACDC data in pull mode (reading last event data from AC/DC RAM).." 
	   << endl << "**" << endl;

      Sumo.check_active_boards(true);

      Sumo.read_CC(true, true, 100,99);
 
    }
    else if(argc == NUM_ARGS+1){
      cout << "invalid argugments, nothing was done " << endl;
      Sumo.sys_wait(1000);
      Sumo.dump_data();
      return -2;
    }

    else{
      cout << "**reading ACDC data in std mode.." 
	   << endl << "**" << endl;
      int device = 0;
      Sumo.read_CC(true, true, device);    

      int mode = Sumo.check_readout_mode();
      if(mode == 1 && Sumo.check_active_boards_slaveDevice() > 0){
	cout << "Slave board detected " << endl;
	device = 1;
	Sumo.read_CC(true, true, device,0);
	
      }
    }
  
    Sumo.sys_wait(1000);
    Sumo.dump_data();
    return 0;
  }
} 
            
