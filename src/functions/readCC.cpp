/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// readCC.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      1;
const char* filename =     "readCC";
const char* description =  "show central card status";

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
  else if(argc == NUM_ARGS+1 && std::string(argv[1]) == "sync"){
    SuMo Sumo;
    int num_checks = 3;    
    
    if(Sumo.check_active_boards(num_checks)) return 1;
    Sumo.check_active_boards(true);

    Sumo.read_CC(true, false, 100);
    return 0;
  }
  /* function defined below */
  else{
    SuMo Sumo;
    int num_checks = 3;    
    
    if(Sumo.check_active_boards(num_checks)) return 1;
    
    Sumo.check_active_boards(true);
    int device = 0;
    Sumo.read_CC(true, false, device);
    
    int mode = Sumo.check_readout_mode();
    if(mode == 1 && Sumo.check_active_boards_slaveDevice() > 0){
      cout << "Slave board detected " << endl;
      device = 1;
      Sumo.read_CC(true, false, device);
    }

    return 0;   
  }
} 
            
