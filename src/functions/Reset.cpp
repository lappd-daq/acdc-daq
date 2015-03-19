/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// resetACDC.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      1;
const char* filename =     "Reset";
const char* description =  "global reset of front-end cards";

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
  else if(argc == NUM_ARGS+1 && std::string(argv[1]) == "hard" ){
    SuMo Sumo;
    for(int ii=0; ii<3; ++ii){
      Sumo.hard_reset(true);
      usleep(1e5);
    }
    return 2;
  }
  else if(argc == NUM_ARGS+1 && std::string(argv[1]) == "time" ){
    SuMo Sumo;
    for(int ii=0; ii<3; ++ii){
      Sumo.reset_time_stamp();
      usleep(1e4);
    }
    return 2;
  }
  else if(argc == NUM_ARGS+1 && std::string(argv[1]) == "usb" ){
    SuMo Sumo;
    for(int ii=0; ii<3; ++ii){
      Sumo.usb_force_wakeup();
      usleep(1e6);
    }
    return 3;
  }
  /* function defined below */
  else{
    SuMo Sumo;
    int num_checks = 5;
    
    Sumo.set_usb_read_mode(16); 
    if(Sumo.check_active_boards(num_checks))
      return 1;

    Sumo.reset_acdc();
    return 0;
    
  }
} 
            
