/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// ledEn.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      2;
const char* filename =     "ledEn";
const char* description =  "turn on/off led's on ACDC card";

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

    if( atoi(argv[1]) == 1 || std::string(argv[1]) == "ON" || 
	std::string(argv[1]) == "on")
      Sumo.toggle_LED(true);
    else if ( atoi(argv[1]) == 0 || std::string(argv[1]) == "OFF" || 
	std::string(argv[1]) == "off")
      Sumo.toggle_LED(false);
    else{
      cout << "invalid argument" << endl;
      return 1;
    }       
    return 0;
  }
} 
            
