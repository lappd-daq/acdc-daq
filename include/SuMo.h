///////////////////////////////////////////////////////////////////
/*
definitions for LAPPD PSEC4 DAQ readout
updated 31-10-2014
author: eric oberla
*/
///////////////////////////////////////////////////////////////////

#ifndef SUMO_H
#define SUMO_H

#include <stdlib.h>
#include <stdio.h>
#include "stdUSB.h"

#define cc_buffersize         31
#define ac_buffersize         8100
#define all_ac_buffersize     24400
#define AC_CHANNELS           30
#define psec_buffersize       1536
#define infoBuffersize        16
#define USB_READ_OFFSET       13
#define psecSampleCells       256
#define numChipsOnBoard       5

#define WRAP_CONSTANT         90
#define numFrontBoards        4

class SuMo{

 public:
  SuMo();
  ~SuMo();

  void sync_usb(bool SYNC);
  void software_trigger(unsigned int SOFT_TRIG_MASK);
  void reset_dll();
  void reset_self_trigger();
  void align_lvds();
  void set_self_trigger(bool ENABLE_TRIG, bool SYS_TRIG_OPTION, bool RATE_ONLY, bool TRIG_SIGN);
  void set_self_trigger_mask(int mask);
  void set_pedestal_value(unsigned int PED_VALUE);
  void set_dll_vdd(unsigned int VALUE);
  void set_trig_threshold(unsigned int TRIG_VALUE);
  void set_ro_target_count(unsigned int TARGET_RO_COUNT);
  void set_usb_read_mode(unsigned int READ_MODE);
  void manage_cc_fifo(bool VALUE);
  void toggle_LED(bool EN);
  void toggle_CAL(bool EN);
  
  int check_usb();
  int read_CC(bool SHOW_CC_STATUS,bool SHOW_AC_STATUS);
  int read_AC(bool ENABLE_FILESAVE, unsigned int trig_mode, int AC_adr);
  int read_ACS(bool ENABLE_FILESAVE);

  int dump_data();
  int get_AC_info(bool PRINT, int AC_adr);
  int generate_ped(bool ENABLE_FILESAVE);
  int make_count_to_voltage(void);
  int load_lut();
  int load_ped();
  int scope_AC(int trig_mode, bool output_mode, int AC_adr);
  int log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate);
  int log_data_hd5(const char* log_filename, unsigned int NUM_READS, int trig_mode, 
		   int acq_rate);

  int check_active_boards(void);
  int check_active_boards(int NUM);

  stdUSB usb;
 
 private:
  static int compare ( const void * a, const void * b){
    return(*(unsigned short*)a - *(unsigned short*)b);
  }
  int unwrap(int ASIC);
  void unwrap_baseline(int *baseline, int ASIC);

  /* metadata from CC */
  unsigned int   CC_INFO[cc_buffersize];  
  unsigned int LAST_CC_INSTRUCT;
  unsigned long long CC_TIMESTAMP;
  unsigned int CC_EVENT_COUNT;
  unsigned int CC_BIN_COUNT;
  unsigned int CC_EVENT_NO;
  bool DC_ACTIVE[numFrontBoards];
  
  /* calibration arrays */
  unsigned int PED_DATA[numFrontBoards][AC_CHANNELS][psecSampleCells];
  float PED_STDEV[numFrontBoards][AC_CHANNELS][psecSampleCells];
  float LUT[numFrontBoards][4096][AC_CHANNELS];
  float oldLUT[numFrontBoards][4096][AC_CHANNELS];
   
  /* waveform and meta-data arrays from ADC boards */
  struct data_t{
    unsigned int LAST_AC_INSTRUCT;
    unsigned int LAST_LAST_AC_INSTRUCT;
    unsigned int EVENT_COUNT;
    unsigned long long int TIMESTAMP;
    float VBIAS[numChipsOnBoard];
    float RO_CNT[numChipsOnBoard];
    float RO_TARGET_CNT[numChipsOnBoard];
    float RO_DAC_VALUE[numChipsOnBoard];
    int   LAST_SAMPLING_BIN[numChipsOnBoard];
    float TRIGGER_THRESHOLD[numChipsOnBoard];
    bool  SELF_TRIG_MODE[numChipsOnBoard];
    bool  TRIG_SIGN[numChipsOnBoard];
    bool  SAMPLING_SPEED[numChipsOnBoard];
    int trigMode;
    int trigSign;
    
    /* raw data saved here */
    unsigned short AC_RAW_DATA[numChipsOnBoard][psec_buffersize];
    unsigned short AC_INFO[numChipsOnBoard][infoBuffersize];
  } acdcData [numFrontBoards];
  
};       
#endif
