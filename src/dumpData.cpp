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
const char* filename =     "dumpData";
const char* description =  "pull";

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
    Sumo.read_CC(false, false);
    Sumo.read_CC(false, false);
    Sumo.sys_wait(1000);
    Sumo.cleanup();
   
    return 0;
    
  }
} 
            
