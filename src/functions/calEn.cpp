/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// calEn.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      3;
const char* filename =     "calEn";
const char* description =  "turn cal switch on/off on ACDC card";

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
    int num_checks = 10;
    Sumo.set_usb_read_mode(16); 
    if(Sumo.check_active_boards(num_checks))
      return 1;
 

    Sumo.set_usb_read_mode(0);
    int device = atoi(argv[2]);
    if(device == 1) Sumo.set_usb_read_mode_slaveDevice(0);

    if( atoi(argv[1]) == 1 || std::string(argv[1]) == "ON" || 
	std::string(argv[1]) == "on"){
      Sumo.toggle_CAL(true, device);
    }
    else if ( atoi(argv[1]) == 0 || std::string(argv[1]) == "OFF" || 
	      std::string(argv[1]) == "off"){
      Sumo.toggle_CAL(false, device);
  }
    else{
      cout << "invalid argument" << endl;
      return 1;
    }       
    return 0;
  }
} 
            
