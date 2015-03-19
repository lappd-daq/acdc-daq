#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SuMo.h"

/* associated source code: */
#include "calibration/makePedandLin.cpp"
#include "calibration/loadPedandLin.cpp"
#include "GetAcdcPackets.cpp"
#include "DAQinstruction.cpp"

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
  
void SuMo::dump_data(void){
  bool all[numFrontBoards];
  for(int i=0; i<numFrontBoards; i++) all[i] = true;
  manage_cc_fifo(1);
  manage_cc_fifo_slaveDevice(1);

  read_AC(1, all,false);

  manage_cc_fifo(1);
  manage_cc_fifo_slaveDevice(1);
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

int SuMo::read_CC(bool SHOW_CC_STATUS, bool SHOW_AC_STATUS, int device){

  bool print = false; // verbose 
  int samples;        // no. of words in usb packet 
  int samples_device_0, samples_device_1;

  if(device == 0)
    for(int i=0; i<4; i++){
      EVENT_FLAG[i] = false;
      DIGITIZING_START_FLAG[i] = false;
    }
  else if(device == 1)
    for(int i=4; i<numFrontBoards; i++){
      EVENT_FLAG[i] = false;
      DIGITIZING_START_FLAG[i] = false;
    }
  else if(device == 100){  //device = 100 for all boards in system
    for(int i=0; i<numFrontBoards; i++){
      EVENT_FLAG[i] = false;
      DIGITIZING_START_FLAG[i] = false;
    }
    if(print) cout << "using device = 100, ALL devices " << endl;
  }
  else return -3; // device assignment error

  /* device = 1 for slave device */
  if(device>1 && mode != USB2x) return -2;
  
  /* set usb read mode for numFrontBoards+1 (central card readout-only) */
  if (device==1)        set_usb_read_mode_slaveDevice(5);
  else if(device == 0)  set_usb_read_mode(5);
  else if(device == 100){
    if(mode==USB2x) set_usb_read_mode_slaveDevice(5);
    set_usb_read_mode(5);
  }
  else return -3;

  int ram_events = 0;
  int num_devices = 1;
  int device_tmp  = device;
  if(device == 100){
    num_devices = 2;
    device_tmp = 0;
  }
  else{
    num_devices = 1;
    device_tmp = device;
  }
  
  for(int loop_cc = device_tmp; loop_cc < device_tmp+num_devices; loop_cc++){
    
    if(print) cout << "device in central card address loop : " << loop_cc << endl;
    
    unsigned short buffer[cc_buffersize];
    memset(buffer, 0x0, cc_buffersize*sizeof(unsigned short));
    /* try readout */
    try{
      if(loop_cc == 1){
	usb2.readData(buffer, cc_buffersize+2, &samples);
	samples_device_1 = samples;
      }
      else if (loop_cc == 0){
	usb.readData(buffer, cc_buffersize+2, &samples); 
	samples_device_0 = samples;
      }
      
      if(samples < 2 && device != 100){
	if(print) cout << "error: no data in buffer on device #" << device << endl;
	return -1;
      }
      int cc_header_found=-1;
      int cc_start_found=-1;
      
      for(int i = 0; i < 5; i++){
	if (buffer[i] == 0x1234){
	  cc_header_found = i;
	  if(print) cout << "cc header @ " << i << endl;
	}
	if (buffer[i] == 0xDEAD){
	  cc_start_found = i;
	  if(print) cout << "cc start @ " << i << endl;
	}
      }
      if(loop_cc == 0) CC_EVENT_COUNT_FROMCC0 = buffer[5];
      if(loop_cc == 1) CC_EVENT_COUNT_FROMCC1 = buffer[5];
        for(int i = 0; i < cc_buffersize; i++){
	CC_INFO[i] = buffer[i];
	if(print) cout << i << ":" << buffer[i] << " ";
      }
      if(print) cout << "samples received: " << samples << " on device #" << device << endl;
    }
    catch(...){
      fprintf(stderr, "Please connect the board. [DEFAULT exception] \n");
      return 1;
    }
  
    if(SHOW_CC_STATUS){  
      cout << endl;
      if(device) cout << "AC/DC connection status :: Slave Device: \n";
      else       cout << "AC/DC connection status: \n";
    }
  
   
    int slave_index = loop_cc*boardsPerCC;
    /* look for ACDC boards, set global variable DC_ACTIVE[numFrontBoards] */
    // add second handle for DC_ACTIVE
    if(buffer[2] & 0x11){ DC_ACTIVE[0+slave_index] = true; 
      if(SHOW_CC_STATUS) cout <<"* DC 0 detected!! \n";}
    if(buffer[2] & 0x22){ DC_ACTIVE[1+slave_index] = true;
      if(SHOW_CC_STATUS) cout <<"* DC 1 detected!! \n";}
    if(buffer[2] & 0x44){ DC_ACTIVE[2+slave_index] = true; 
      if(SHOW_CC_STATUS) cout <<"* DC 2 detected!! \n";}
    if((buffer[2] & 0x8) && (buffer[2] & 0x88)){ DC_ACTIVE[3+slave_index] = true;
      if(SHOW_CC_STATUS) cout <<"* DC 3 detected!! \n";}
    
    //check for events in CC RAM
    if(buffer[4] & 0x1) EVENT_FLAG[0+slave_index] = true; 
    if(buffer[4] & 0x2) EVENT_FLAG[1+slave_index] = true;
    if(buffer[4] & 0x4) EVENT_FLAG[2+slave_index] = true; 
    if(buffer[4] & 0x8) EVENT_FLAG[3+slave_index] = true;
    ram_events += EVENT_FLAG[0+slave_index] + 
      EVENT_FLAG[1+slave_index] + 
      EVENT_FLAG[2+slave_index] + 
      EVENT_FLAG[3+slave_index];
    
    if(buffer[4]>>4 & 0x1) DIGITIZING_START_FLAG[0+slave_index] = true; 
    if(buffer[4]>>4 & 0x2) DIGITIZING_START_FLAG[1+slave_index] = true;
    if(buffer[4]>>4 & 0x4) DIGITIZING_START_FLAG[2+slave_index] = true; 
    if(buffer[4]>>4 & 0x8) DIGITIZING_START_FLAG[3+slave_index] = true;

  
  }  //end loop over central cards
  
  /* print board meta-data */
  if(SHOW_AC_STATUS){
    bool tmp_active[numFrontBoards];
    for(int ii = 0; ii<numFrontBoards; ii++) tmp_active[ii] = false; //mask off master or slave board for printing info

    if(device == 1){     //slave device
      for(int board=boardsPerCC; board<numFrontBoards; board++) tmp_active[board]=DC_ACTIVE[board];

      read_AC(0,tmp_active,false);
      for(int board=boardsPerCC; board<numFrontBoards; board++)
	if(DC_ACTIVE[board]){
	  cout << endl << "AC/DC #" << board << ":";
	  get_AC_info(true, board);
	}
      manage_cc_fifo_slaveDevice(1);
    }
    else if( device == 0){          //master device
      for(int board=0; board<boardsPerCC; board++) tmp_active[board]=DC_ACTIVE[board];

      read_AC(0,tmp_active,false);
      for(int board=0; board<boardsPerCC; board++)
	if(DC_ACTIVE[board]){
	  cout << endl << "AC/DC #" << board << ":";
	  get_AC_info(true, board);
	}
      manage_cc_fifo(1);
    }
    else if( device == 100){          //all devices
      
      read_AC(0,DC_ACTIVE,false, true);
      for(int board=0; board<numFrontBoards; board++)
	if(DC_ACTIVE[board]){
	  cout << endl << "AC/DC #" << board << ":";
	  get_AC_info(true, board);
	}
      if(mode == USB2x) manage_cc_fifo_slaveDevice(1);
      manage_cc_fifo(1);
    }
  }
  
  if(SHOW_CC_STATUS) cout << "\n**********************\n";
  
  //if(device)  usb2.freeHandles();
  //else        usb.freeHandles();

  return ram_events;
}

int SuMo::get_AC_info(bool PRINT, int frontEnd){
  int aa = frontEnd;
  unsigned short AC_INFO[numChipsOnBoard][infoBuffersize];
  for(int i=0; i<numChipsOnBoard; i++){
    for(int j=0; j<infoBuffersize; j++){
      AC_INFO[i][j] = adcDat[aa]->AC_INFO[i][j];
    }
  }
  unsigned short ref_volt_mv  = 1200;
  unsigned short num_bits     = 4096;
  for (int i = 0; i < 5; i++){
    adcDat[aa]->ro_cnt[i] =          (float) AC_INFO[i][4] * 10 * pow(2,11)/ (pow(10,6));
    adcDat[aa]->ro_target_cnt[i] =   (float) AC_INFO[i][5] * 10 * pow(2,11)/(pow(10,6));
    adcDat[aa]->vbias[i] =           (int) AC_INFO[i][6];
    adcDat[aa]->trigger_threshold[i]=(float) AC_INFO[i][7] * ref_volt_mv/num_bits;
    adcDat[aa]->ro_dac_value[i] =    (float) AC_INFO[i][8] * ref_volt_mv/num_bits;
  }
   
  int ab=adcDat[aa]->CC_BIN_COUNT =         (adcDat[aa]->CC_HEADER_INFO[1] & 0x18) >> 3;
  int bb=adcDat[aa]->CC_EVENT_COUNT =       adcDat[aa]->CC_HEADER_INFO[3] | adcDat[aa]->CC_HEADER_INFO[2] << 16;
  int cc=adcDat[aa]->CC_TIMESTAMP_LO =      adcDat[aa]->CC_HEADER_INFO[4];
  int dd=adcDat[aa]->CC_TIMESTAMP_MID =     adcDat[aa]->CC_HEADER_INFO[5];
  int ee=adcDat[aa]->CC_TIMESTAMP_HI =      adcDat[aa]->CC_HEADER_INFO[6]; 

  int ff=adcDat[aa]->bin_count_rise =       AC_INFO[0][9] & 0x0F;
  int gg=adcDat[aa]->bin_count_fall =       (AC_INFO[0][9] & 0xF0) >> 4;

  int hh=adcDat[aa]->self_trig_settings =   AC_INFO[1][9];
  int ii=adcDat[aa]->trig_en =              adcDat[aa]->self_trig_settings & 0x1;
  int jj=adcDat[aa]->trig_wait_for_sys =    adcDat[aa]->self_trig_settings & 0x2;
  int kk=adcDat[aa]->trig_rate_only =       adcDat[aa]->self_trig_settings & 0x4;
  int ll=adcDat[aa]->trig_sign =            adcDat[aa]->self_trig_settings & 0x8;
  
  //acdcData[aa].EVENT_COUNT =          AC_INFO[2][9]  | AC_INFO[2][9]  << 16;
  int mm =adcDat[aa]->reg_self_trig[0] =     AC_INFO[0][10] | AC_INFO[1][10] << 16;
  int nn =adcDat[aa]->reg_self_trig[1] =     AC_INFO[2][10] | AC_INFO[3][10] << 16;
  int oo =adcDat[aa]->reg_self_trig[2] =     AC_INFO[4][10] | AC_INFO[0][11] << 16;
  int pp =adcDat[aa]->reg_self_trig[3] =     AC_INFO[1][11] | AC_INFO[2][11] << 16;
  int qq =adcDat[aa]->self_trig_mask =       AC_INFO[3][11] | AC_INFO[4][11] << 16;
  long rr =adcDat[aa]->last_ac_instruct =     AC_INFO[0][12] | AC_INFO[1][12] << 16 | AC_INFO[2][12] << 32; //now another timestamp
  int rr_a = AC_INFO[0][12];
  int rr_b = AC_INFO[1][12];
  int rr_c = AC_INFO[2][12];
  int ss =adcDat[aa]->last_last_ac_instruct= AC_INFO[3][12] | AC_INFO[4][12] << 16;  //now another event counter
  int tt =adcDat[aa]->event_count =          AC_INFO[3][13] | AC_INFO[4][13] << 16;
  int uu =adcDat[aa]->timestamp_hi=          AC_INFO[2][13];
  int vv =adcDat[aa]->timestamp_mid=         AC_INFO[1][13];
  int ww =adcDat[aa]->timestamp_lo=          AC_INFO[0][13];

  //cout << "dig time: " << rr_c << ":" << rr_b << ":" << rr_a << endl;
  if(PRINT){
    
    cout << std::fixed;
    cout << std::setprecision(2);
    cout << std::dec << endl;
    cout << "central card header info: " << endl;
    cout << "bin:" << ab
	 << " evt.count:" << bb
	 << " timestamp: " <<  ee <<":"<< dd <<":"<< cc << endl;
    cout << "--------" << endl;
    cout << "event count: " << std::dec << tt; 
    cout << " board time: " << uu <<":"<< vv <<":"<< ww << endl;
    cout << "digtz count: " << std::dec << ss; 
    cout << "   ADC time: " << rr_c <<":"<< rr_b <<":"<< rr_a << endl;
    //cout << "last instructions: 0x" <<  std::hex << rr << ", 0x" << ss << endl;
    cout << "registered self-trig bits: 0x" << hex << mm
	 <<", 0x"                           << hex << nn
	 <<", 0x"                           << hex << oo
         <<", 0x"                           << hex << pp
	 << endl;
    cout << "self-trig mask: 0x"     << hex << qq << endl;
    cout << "self-trig settings: 0x" << hex << hh << endl;  
    cout << "trig_sign: "            << ll
	 << ", wait_for_sys: "       << jj
	 << ", rate_only: "          << kk
	 << ", EN: "                 << ii
	 << endl;
    cout << "bin count rise edge: "  << ff
	 << ", bin count fall edge: "<< gg
         << endl;

    for (int i = 0; i < 5; i++){
      cout << "PSEC:" << i;
      cout << "|ADC clock/trgt:"     << adcDat[aa]->ro_cnt[i];
      cout << "/"                    << adcDat[aa]->ro_target_cnt[i] << "MHz";
      cout << ",bias:"               << adcDat[aa]->ro_dac_value[i] <<"mV";
      cout << "|Ped:"           <<dec<< adcDat[aa]->vbias[i]* ref_volt_mv/num_bits << "mV";
      cout << "|Trig:"               << adcDat[aa]->trigger_threshold[i] << "mV";
      cout << endl;
    }
  } 
  return 0;
}

//unwrap locally on AC/DC
/*
int SuMo::unwrap(int ASIC){
  int last_sampling_bin = LAST_SAMPLING_BIN[ASIC];
  unsigned int BIN_40[4];

  for(int k=0; k<4; k++){
    BIN_40[k] = (int)256/4*k;
  }

  return BIN_40[last_sampling_bin];
}
*/

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
