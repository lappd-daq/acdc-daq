/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// resetDll.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      1;
const char* filename =     "resetDll";
const char* description =  "reset sampling lock on front-end cards";

using namespace std;

int main(int argc, char* argv[]){
 
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1; 
  }
  else if(argc > NUM_ARGS + 1){
    cout << "error: wrong number of arguments" << endl;
    return -1;
  }
  /* function defined below */
  else if(argc == NUM_ARGS + 1 && (std::string(argv[1]) == "-sync" ||
				   std::string(argv[1]) == "-s")){
    SuMo Sumo;
    int num_checks = 5;
    Sumo.set_usb_read_mode(16); 
    Sumo.set_usb_read_mode_slaveDevice(16);
    if(Sumo.check_active_boards(num_checks))
      return 1;

    Sumo.reset_dll(true);
    cout << "resetting PSEC4 sampling...in sync mode\n";
    return 0;
  }
  else{
    SuMo Sumo;
    int num_checks = 5;
    
    Sumo.set_usb_read_mode(16); 
    if(Sumo.check_active_boards(num_checks))
      return 1;

    Sumo.reset_dll();
    cout << "resetting PSEC4 sampling...\n";
    return 0;
    
  }
} 
            
