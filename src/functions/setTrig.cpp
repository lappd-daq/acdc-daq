/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// setTrig.cpp
// written: ejo
/////////////////////////////////
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <sstream>
#include "SuMo.h"

using namespace std;

/* specific to file */
const int NUM_ARGS  =    2;
const char* filename =     "setTrig";
const char* description=   "setup trigger, parse trigger parms file (argv[1])";

unsigned int trig_mask[numFrontBoards];
bool         trig_enable[numFrontBoards];
bool         trig_sign;             // (-) pulses
bool         wait_for_sys;
bool         rate_only;   
bool         hrdw_trig;   
/* running this function without paramsFile, turns off all board-level triggers */

void setDefaultValues();
int parseTrigParams(const char* file);

int main(int argc, char* argv[]){
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1;
  }
  else if(argc > NUM_ARGS){
    cout << "error: wrong number of arguments" << endl;
    return -1;
  }
  /* function defined below */
  else{
    SuMo Sumo;
    int num_checks = 5;
    char paramsFile[200];

    if(Sumo.check_active_boards(num_checks)) return 1;

    if(argc == 2){
      strcpy(paramsFile, argv[1]);
      parseTrigParams(paramsFile);
    }
    else  
      setDefaultValues();

    //Sumo.reset_self_trigger();
    /* send trig mask to boards in 2 sets of 16 bit words */
    Sumo.set_self_trigger_mask(0x00007FFF&trig_mask[0], 0);
    Sumo.set_self_trigger_mask(0x3FFF8000&trig_mask[0], 1);
    /* push trigger settings, can check by reading back ACDC settings */
    Sumo.set_self_trigger(trig_enable[0], wait_for_sys, rate_only, trig_sign);
   
    //Sumo.dump_data();
    return 0;
  }
}

/* default trig settings (OFF) */
void setDefaultValues(){
  for(int i=0; i<numFrontBoards; i++){
    trig_mask[i]   = 0x00000000;  // 32 bit
    trig_enable[i] = false;
  }
  trig_sign   = 1;   // 1 = (-polarity), 0 = (+) 
  wait_for_sys= false;
  rate_only   = false;
  hrdw_trig   = false;
  cout << "self-trigger disabled" << endl;
}
/* parse parameter file */
int parseTrigParams(const char* file){

  ifstream in;
  in.open(file, ios::in);
  string line, data;
  
  unsigned int tmp1, tmp2, tmp3;
  
  while(getline(in,line)){
    stringstream linestream(line);
    getline(linestream, data, '\t');

    if(data.find("#")==0)
      continue;

    if(data.find("trig_mask")==0){
      linestream >> tmp1 >> hex >> tmp2;
      trig_mask[(int)tmp1] = tmp2;
      cout << data << " on board " << tmp1 << " set to 0x" 
	   << hex << tmp2 << endl;
    }      
    else if(data.find("trig_enable")==0){
      linestream >> tmp1 >> tmp2;
      trig_enable[(int)tmp1] = (bool)tmp2;
      cout << data << " on board " << tmp1 << " set to " << tmp2 << endl;
    } 
    else if(data.find("trig_sign")==0){
      linestream >> tmp1;
      trig_sign = (bool)tmp1;  
      cout << data << " set to " << tmp1 << endl;
    } 
    else if(data.find("wait_for_sys")==0){
      linestream >> tmp1;
      wait_for_sys = (bool)tmp1;
      cout << data << " set to " << tmp1 << endl;
    } 
    else if(data.find("rate_only")==0){
      linestream >> tmp1;
      rate_only = (bool)tmp1;
      cout << data << " set to " << tmp1 << endl;
    } 
    else if(data.find("hrdw_trig")==0){
      linestream >> tmp1;
      hrdw_trig= (bool)tmp1;
      cout << data << " set to " << tmp1 << endl;
    } 
  }
  return 0;
}  
