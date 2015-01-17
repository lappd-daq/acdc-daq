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
#include <math.h>
#include "SuMo.h"

using namespace std;

/* specific to file */
const int NUM_ARGS  =    2;
const char* filename =     "setTrig";
const char* description=   "setup trigger, parse trigger parms file (argv[1])";

//configuration variables:
unsigned int trig_mask[numFrontBoards];
bool         trig_enable[numFrontBoards];
unsigned int pedestal[numFrontBoards][numChipsOnBoard];
unsigned int threshold[numFrontBoards][numChipsOnBoard];
unsigned int cal_en[numFrontBoards];
bool         trig_sign;             
bool         wait_for_sys;
bool         rate_only;   
bool         sma_trig_on_fe[numFrontBoards];
bool         hrdw_trig;
bool         hrdw_trig_sl;
unsigned int hrdw_trigsrc;
unsigned int hrdw_trig_slsrc;

/* running this function without paramsFile, turns off all board-level triggers */

void set_default_values();
int  parse_setup_file(const char* file, bool verbose=false);
int  write_config_to_hardware(SuMo&);

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
    int  num_checks = 5;
    char paramsFile[200];

    if(Sumo.check_active_boards(num_checks)) return 1;

    if(argc == 2){
      strcpy(paramsFile, argv[1]);
      parse_setup_file(paramsFile); 
    }
    else  
      set_default_values();
            
    write_config_to_hardware(Sumo);

    return 0;
  }
}

/* default trig settings (OFF) */
void set_default_values(){
  for(int i=0; i<numFrontBoards; i++){
    for(int j=0; j<numChipsOnBoard; j++){
      pedestal[i][j]       = 0x800;
      threshold[i][j]      = 0x000;
    }
    
    trig_mask[i]      = 0x00000000;  // 32 bit
    trig_enable[i]    = false;
    sma_trig_on_fe[i] = false;

  }
  trig_sign      = 0;   
  wait_for_sys   = false;
  rate_only      = false;
  hrdw_trig      = false;
  hrdw_trig_sl   = false;
  hrdw_trigsrc   = 0;
  hrdw_trig_slsrc= 0;
  cout << "self-trigger disabled" << endl;
}

/* write registers */
int write_config_to_hardware(SuMo& Sumo){
  int device       = 0;

  cout << "__________________________" << endl;

  Sumo.set_usb_read_mode(16);
    
  int mode = Sumo.check_readout_mode();
  if(mode == 1 && Sumo.check_active_boards_slaveDevice() > 0){
    Sumo.set_usb_read_mode_slaveDevice(16);
  }
  
  Sumo.dump_data();
  
  for(int i=0; i<numFrontBoards; i++){
    //Sumo.reset_self_trigger();
    unsigned int boardAddress = pow(2, i % 4);
    if(i >= 4)   device = 1;  
    
    if(Sumo.DC_ACTIVE[i] == false){
      cout << "no board = " << i << " addressed at device " 
	   << device << ":0x" << hex << boardAddress << endl;
      cout << "__________________________" << std::dec << endl;
      continue;
    }
    
    cout << "writing settings to board " << i << " addressed at device " 
	 << device << ":0x" << hex << boardAddress << endl;
    
    /* send trig mask to boards in 2 sets of 16 bit words */
    Sumo.set_self_trigger_mask(0x00007FFF&trig_mask[i], 0, boardAddress, device);
    Sumo.set_self_trigger_mask((0x3FFF8000&trig_mask[i]) >> 15, 1, boardAddress, device);
    
    /* push trigger settings, can check by reading back ACDC settings */
    cout << "writing trig_enable of " << trig_enable[i] << " to board " << i << endl;
      
    for(int j=0; j<numChipsOnBoard; j++){
      unsigned int chipAddress = pow(2, j % numChipsOnBoard);

      Sumo.set_pedestal_value(pedestal[i][j], boardAddress, device, chipAddress);
      Sumo.set_trig_threshold(threshold[i][j], boardAddress, device, chipAddress);
    }

    Sumo.set_self_trigger( trig_enable[i], 
			   wait_for_sys, 
			   rate_only, trig_sign, 
			   sma_trig_on_fe[i], 
			   boardAddress, 
			   device);
    
    cout << "__________________________" << std::dec << endl;
    //Sumo.dump_data();
  }
  
  if(hrdw_trig){
    unsigned int ext_trig_mode = 0x0 | 1 << 3 | 1 << 4 | hrdw_trigsrc << 12;
    cout << "setting trig mode to master device " << ext_trig_mode << endl;
    Sumo.set_usb_read_mode(ext_trig_mode);
  }
  
  if(hrdw_trig_sl){
    unsigned int ext_trig_mode = 0x0 | 1 << 3 | 1 << 4 | hrdw_trig_slsrc << 12;
    cout << "setting trig mode to slave device " << ext_trig_mode << endl;
    Sumo.set_usb_read_mode_slaveDevice(ext_trig_mode);
  }

  return 0;
}

  
/* parse parameter file */
int parse_setup_file(const char* file, bool verbose){
  bool tt = verbose;

  ifstream in;
  in.open(file, ios::in);
  string line, data;
  
  unsigned int tmp1, tmp2, tmp3;
  bool bool_tmp1;

  while(getline(in,line)){
    stringstream linestream(line);
    getline(linestream, data, '\t');

    if(data.find("#")==0)
      continue;

    if(data.find("trig_mask")==0){
      linestream >> tmp1 >> hex >> tmp2;
      trig_mask[tmp1] = tmp2;
      if(tt) 
	cout << data << " on board " << tmp1 << " set to 0x" << hex << tmp2 << dec << endl;
    }      
    else if(data.find("trig_enable")==0){
      linestream >> tmp1 >> bool_tmp1;
      trig_enable[tmp1] = bool_tmp1;
      if(tt)
	cout << data << " on board " << tmp1 << " set to " << bool_tmp1 << endl;
    } 
    else if(data.find("trig_sign")==0){
      linestream >> bool_tmp1;
      trig_sign = bool_tmp1;  
      if(tt)
	cout << data << " set to " << bool_tmp1 << endl;
    } 
    else if(data.find("wait_for_sys")==0){
      linestream >> bool_tmp1;
      wait_for_sys = bool_tmp1;
      cout << data << " set to " << bool_tmp1 << endl;
    } 
    else if(data.find("rate_only")==0){
      linestream >> bool_tmp1;
      rate_only = bool_tmp1;
      if(tt)
	cout << data << " set to " << bool_tmp1 << endl;
    } 
    else if(data.find("hrdw_trig")==0){
      linestream >> bool_tmp1;
      hrdw_trig = bool_tmp1;
      if(tt)
	cout << data << " set to " << hrdw_trig << endl;
    } 
    else if(data.find("hrdw_sl_trig")==0){
      linestream >> bool_tmp1;
      hrdw_trig_sl = bool_tmp1;
      if(tt)
	cout << data << " set to " << bool_tmp1 << endl;
    } 
    else if(data.find("hrdw_src_trig")==0){
      linestream >> tmp1;
      hrdw_trigsrc = tmp1;
      cout << data << " set to " << tmp1 << endl;
    } 
    else if(data.find("hrdw_slsrc_trig")==0){
      linestream >> tmp1;
      hrdw_trig_slsrc = tmp1;
      if(tt)
	cout << data << " set to " << tmp1 << endl;
    } 
   else if(data.find("sma_trig_on_fe")==0){
      linestream >> tmp1 >> bool_tmp1;
      sma_trig_on_fe[tmp1]= bool_tmp1;
      if(tt)
	cout << data << " on board " << tmp1 << " set to " << bool_tmp1 << endl;
    } 
    else if(data.find("pedestal")==0){
      linestream >> tmp1 >> tmp3 >>  tmp2;
      pedestal[tmp1][tmp3]= tmp2;
      if(tt)
	cout << data << " on board:chip " << tmp1 << ":" << tmp3 << " set to 0x"  << hex << tmp2 << endl;
    }
    else if(data.find("thresh")==0){
      linestream >> tmp1 >> tmp3 >> tmp2;
      threshold[tmp1][tmp3]= tmp2;
      if(tt)
	cout << data << " on board:chip " << tmp1 << ":" << tmp3 << " set to 0x"  << hex << tmp2 << endl;
    }

  }
  return 0;
}  
