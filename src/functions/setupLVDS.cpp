/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// setupLVDS.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS =      1;
const char* filename =     "setupLVDS";
const char* description =  "setup lvds SERDES interface";

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
    SuMo sumo;
    sumo.align_lvds();
    cout << "aligning LVDS SERDES interface...\n";
    return 0;    
  }
} 
            
