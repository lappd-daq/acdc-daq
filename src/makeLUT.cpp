/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// makeLUT.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS  =    1;
const char* filename =     "makeLUT";
const char* description=   "generate count-to-voltage conversion files for active boards";

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
    int num_checks = 5;
    
    if(Sumo.check_active_boards(num_checks))
      return 1;

    Sumo.set_usb_read_mode(16);
    Sumo.dump_data();
    Sumo.make_count_to_voltage();
    return 0;
  }
}
