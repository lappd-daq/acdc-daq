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
#define infoBuffersize        17
#define USB_READ_OFFSET       13
#define psecSampleCells       256
#define numChipsOnBoard       5

#define WRAP_CONSTANT         90
#define numFrontBoards        4

#define usbPacketStart        0x1234
#define usbPacketEnd          0x4321 
#define dataPacketStart       0xF005
#define adcPacketEnd          0xBA11
#define dataPacketEnd         0xFACE

class SuMo{

 public:
  SuMo();
  ~SuMo();

  void sync_usb(bool SYNC);
  void software_trigger(unsigned int SOFT_TRIG_MASK);
  void reset_dll();
  void reset_self_trigger();
  void reset_time_stamp();
  void reset_acdc();
  void align_lvds();
  void set_self_trigger(bool ENABLE_TRIG, bool SYS_TRIG_OPTION, bool RATE_ONLY, bool TRIG_SIGN);
  void set_self_trigger_mask(int mask, bool HILO);
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
  int read_AC(bool ENABLE_FILESAVE, unsigned int trig_mode, int AC_adr); /*deprecated, phase this one out */
  /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for additional retval handling: */
  int read_AC(unsigned int trig_mode, bool* mask, bool FILESAVE);  /* use this bulk read function instead */

  void dump_data(){
    bool all[numFrontBoards] = {1,1,1,1};
    read_AC(1, all,false);
    manage_cc_fifo(1);
  }
  int get_AC_info(bool PRINT, int AC_adr);
  int generate_ped(bool ENABLE_FILESAVE);
  int make_count_to_voltage(void);
  int load_lut();
  int load_ped();
  int oscilloscope(int trig_mode, int numFrames, int AC_adr, int range[2]);
  int log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate);
  int log_data_hd5(const char* log_filename, unsigned int NUM_READS, int trig_mode, 
		   int acq_rate);
  void form_meta_data(int Address, int count, double time);

  int            check_active_boards(void);
  int            check_active_boards(int NUM);
  bool           DC_ACTIVE[numFrontBoards];
  bool           BOARDS_READOUT[numFrontBoards];
  bool           BOARDS_TIMEOUT[numFrontBoards];

  stdUSB usb;

  /* waveform and meta-data arrays from ADC boards */
  struct data_t{
    unsigned int           LAST_AC_INSTRUCT;
    unsigned int           LAST_LAST_AC_INSTRUCT;
    unsigned int           EVENT_COUNT;
    unsigned int           TIMESTAMP_HI; /* 16 bits */
    unsigned int           TIMESTAMP_MID;/* 16 bits */
    unsigned int           TIMESTAMP_LO; /* 16 bits */
    float                  VBIAS[numChipsOnBoard];
    float                  RO_CNT[numChipsOnBoard];
    float                  RO_TARGET_CNT[numChipsOnBoard];
    float                  RO_DAC_VALUE[numChipsOnBoard];
    int                    LAST_SAMPLING_BIN[numChipsOnBoard];
    float                  TRIGGER_THRESHOLD[numChipsOnBoard];
    bool                   TRIG_SIGN;
    bool                   TRIG_RATE_ONLY;
    bool                   TRIG_WAIT_FOR_SYS;
    bool                   TRIG_EN;
    unsigned int           BIN_COUNT_RISE;
    unsigned int           BIN_COUNT_FALL;
    unsigned int           SELF_TRIG_SETTINGS;
    unsigned int           REG_SELF_TRIG[4];
    unsigned int           SELF_TRIG_MASK;
    unsigned int           SELF_TRIG_SCALER[AC_CHANNELS];  

    unsigned int           PKT_HEADER;
    unsigned int           PKT_FOOTER;
    
    unsigned int           DATA_HEADER[numChipsOnBoard];
    unsigned int           DATA_ADC_END[numChipsOnBoard];
    unsigned int           DATA_FOOTER[numChipsOnBoard];
    /* raw data saved here */
    unsigned short         AC_RAW_DATA[numChipsOnBoard][psec_buffersize];
    float                  Data[AC_CHANNELS+1][psecSampleCells]; /* AC_CHANNELS waveforms + 1 metadata */
    unsigned short         AC_INFO[numChipsOnBoard][infoBuffersize];
  } acdcData [numFrontBoards];
 
 private:
  static int compare ( const void * a, const void * b){
    return(*(unsigned short*)a - *(unsigned short*)b);
  }
  int unwrap(int ASIC);
  void unwrap_baseline(int *baseline, int ASIC);

  /* metadata from CC */
  unsigned int       CC_INFO[cc_buffersize];  
  unsigned int       LAST_CC_INSTRUCT;
  unsigned long long CC_TIMESTAMP;
  unsigned int       CC_EVENT_COUNT;
  unsigned int       CC_BIN_COUNT;
  unsigned int       CC_EVENT_NO;
  
  /* calibration arrays */
  unsigned int PED_DATA[numFrontBoards][AC_CHANNELS][psecSampleCells];
  float        PED_STDEV[numFrontBoards][AC_CHANNELS][psecSampleCells];
  float        LUT[numFrontBoards][4096][AC_CHANNELS];
  float        oldLUT[numFrontBoards][4096][AC_CHANNELS]; 
  
};       
#endif
