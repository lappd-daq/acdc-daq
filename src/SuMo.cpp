#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <sstream>
#include <math.h>
#include <signal.h>
#include <sys/stat.h>
#include "SuMo.h"

/* associated source code: */
#include "calibration/makePedandLin.cpp"
#include "calibration/loadPedandLin.cpp"
#include "GetAcdcPackets.cpp"
#include "GetSysPackets.cpp"
#include "DAQinstruction.cpp"
#include "form_meta_data.cpp"

using namespace std;

SuMo::SuMo()
{
  check_readout_mode();
  for(int i=0; i<numFrontBoards; i++){
    DC_ACTIVE[i] = false;
    EVENT_FLAG[i] = false;
    adcDat[i] = new packet_t();
  }
}

SuMo::~SuMo()
{
  dump_data();
  for(int i=0; i<numFrontBoards; i++) delete [] adcDat[i];

}
void SuMo::cleanup(){
  for(int jj=0; jj<2; jj++){
    if(mode==USB2x) system_slave_card_trig_valid(false);
    system_card_trig_valid(false);
    if(mode==USB2x) manage_cc_fifo_slaveDevice(1);
    manage_cc_fifo(1);
    //
    dump_data();
    if(mode==USB2x) set_usb_read_mode_slaveDevice(16), set_usb_read_mode_slaveDevice(0);
    set_usb_read_mode(16), set_usb_read_mode(0);
    //
    dump_data();
  }
  //for(int i=0; i<numFrontBoards; i++) delete [] adcDat[i];
}

void SuMo::dump_data(void){
  bool all[numFrontBoards];
  for(int i=0; i<numFrontBoards; i++) all[i] = true;
  manage_cc_fifo(1);
  if(mode == USB2x) manage_cc_fifo_slaveDevice(1);
  read_AC(1, all,false);
  manage_cc_fifo(1);
  if(mode == USB2x) manage_cc_fifo_slaveDevice(1);
}

int SuMo::check_readout_mode(void){
  if(usb2.createHandles() == stdUSB::SUCCEED && usb.createHandles() == stdUSB::SUCCEED){
    mode = USB2x;
    usb.freeHandles();
    usb2.freeHandles();
    //cout << "mode is " << mode << endl;
  }
  else if(usb.createHandles() == stdUSB::SUCCEED){
    mode = USB;
    usb.freeHandles();
  }
  else
    mode = UNK;
  return mode;
}

int SuMo::check_active_boards(bool print){

  int num_boards_active = 0;
  for(int targetAC = 0; targetAC < numFrontBoards; targetAC++){
    if(DC_ACTIVE[targetAC] == true){
      num_boards_active ++;
      if(print) cout << "Board at address " << targetAC << " of " << numFrontBoards-1 << " found" << endl;
    }
  }
  return num_boards_active;
}
/* slave USB device checks */
int SuMo::check_active_boards_slaveDevice(void){

  int num_boards_active = 0;
  for(int targetAC = 4; targetAC < numFrontBoards; targetAC++){
    if(DC_ACTIVE[targetAC] == true){
      num_boards_active ++;
    }
  }
  return num_boards_active;
}

int SuMo::check_active_boards(int NUM){

  for(int i=0; i<NUM; i++){
    int device = 0;
    read_CC(false, false, device);
    if(mode==USB2x){
      device=1;
      read_CC(false, false, device);
    }
  }

  if(check_active_boards(false) == 0){
      cout << "failed to find connected ADC boards" << endl;
      return 1;
  }
  //cout << "readout mode is " << mode << endl;
  return 0;
}
bool SuMo::fileExists(const string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}
int SuMo::unwrap(int ASIC){
  int last_sampling_bin = CC_BIN_COUNT;
  //int last_sampling_bin = 0;
  unsigned int BIN_40[4];

  //note BIN_40 assuming 160MHz triggering clock

  for(int k=0; k<4; k++){
    BIN_40[k] = (int)256/4*k;
  }
  return BIN_40[last_sampling_bin];
}

void SuMo::unwrap_baseline(int *baseline, int ASIC){
  int start_baseline[256];
  for(int i = 0; i < 256; i++){
    start_baseline[i] = i;
  }

  int wraparound = unwrap(ASIC);

  if((wraparound+WRAP_CONSTANT) < 257)
    wraparound = wraparound+WRAP_CONSTANT;
  else
    wraparound = wraparound+WRAP_CONSTANT-256;

  //printf("%d", wraparound);
  for(int i = 0; i < 256; i++){
    baseline[i] = start_baseline[ (wraparound + i) % 256];
  }
}
