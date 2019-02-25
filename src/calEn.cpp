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
const char* arg_desc = "<on/off> <board> [<hex channel code>]"

using namespace std;

int main(int argc, char* argv[]){
 

 //check for right arguments
  if((argc != 3 && argc != 2) && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    cout << "usage: " << "./bin/" << filename << " " << arg_desc << endl;
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
    //open USB line and check for boards a number of times
    if(Sumo.check_active_boards(num_checks))
      return 1;
 


    Sumo.set_usb_read_mode(0);

    //the board number for the action
    int device = atoi(argv[2]);
    char* temp_ptr; 

    //convert 3rd argument to an unsigned int mask. This means
    // the 3rd argument should be of the format 0x7FFF 
    unsigned int channels;
    if(argc == 2)
    {
      channels = 0x7FFF;
    }
    else
    {
      channels = strtoul(argv[3], &temp_ptr, 16);
    }

    //Evan doesn't understand this device == 1, thought 
    //device was the board number...
    if(device == 1) Sumo.set_usb_read_mode_slaveDevice(0);
    if( atoi(argv[1]) == 1 || std::string(argv[1]) == "ON" || std::string(argv[1]) == "on"){
      Sumo.toggle_CAL(true, device, channels);
    }
    else if ( atoi(argv[1]) == 0 || std::string(argv[1]) == "OFF" || 
	      std::string(argv[1]) == "off"){
      Sumo.toggle_CAL(false, device, channels);
  }
    else{
      cout << "invalid argument" << endl;
      return 1;
    }       
    return 0;
  }
} 
            
