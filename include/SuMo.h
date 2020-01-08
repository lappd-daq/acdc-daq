///////////////////////////////////////////////////////////////////
/*
definitions and prototypes for LAPPD PSEC4 DAQ readout
updated 31-10-2014
author: eric oberla
*/
///////////////////////////////////////////////////////////////////

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <string>

#include "stdUSB.h"
#include "Definitions.h"
#include "Packet.h"

enum cc_readout_mode {
  USB = 0, USB2x = 1, ETH = 2, UNK = 3
};

class SuMo {

public:
  SuMo(); //SuMo.cpp

  ~SuMo();//SuMo.cpp

  void sync_usb(bool SYNC); //DAQinstruction.cpp

  //instructions to hardware
  void reset_dll(bool sync = false); //DAQinstruction.cpp

  void reset_self_trigger(unsigned int mask, int device = 0); //DAQinstruction.cpp

  void reset_time_stamp(bool sync = false); //DAQinstruction.cpp

  void reset_acdc(); //DAQinstruction.cpp

  void align_lvds(); //DAQinstruction.cpp

  //front-end card specific instructions
  void toggle_LED(bool EN); //DAQinstruction.cpp

  void toggle_CAL(bool EN, int device = 0, unsigned int channels=0x7FFF); //DAQinstruction.cpp

  void set_self_trigger_lo(bool ENABLE_TRIG, bool SYS_TRIG_OPTION,
                           bool RATE_ONLY, bool TRIG_SIGN, bool USE_SMA,
                           bool USE_COINCIDENCE, bool USE_FAST_COINCIDENCE,
                           unsigned int coinc_window,
                           unsigned int mask, int device = 0); //DAQinstruction.cpp

  void set_self_trigger_hi(unsigned int coinc_pulse_width,
                           unsigned int asic_coincidence_min,
                           unsigned int channel_coincidence_min,
                           unsigned int mask, int device = 0); //DAQinstruction.cpp

  void set_self_trigger_mask(int mask, bool HILO, unsigned int board_mask, int device = 0); //DAQinstruction.cpp

  //chip-level instructions
  void set_pedestal_value(unsigned int PED_VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31); //DAQinstruction.cpp

  void set_dll_vdd(unsigned int VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31); //DAQinstruction.cpp

  void set_trig_threshold(unsigned int TRIG_VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31); //DAQinstruction.cpp

  void
  set_ro_target_count(unsigned int TARGET_RO_COUNT, unsigned int mask, int device = 0, unsigned int psec_mask = 31); //DAQinstruction.cpp

  //manage PC interface
  void set_usb_read_mode(unsigned int READ_MODE); //DAQinstruction.cpp

  void set_usb_read_mode_slaveDevice(unsigned int READ_MODE); //DAQinstruction.cpp

  void manage_cc_fifo(bool VALUE); //DAQinstruction.cpp

  void manage_cc_fifo_slaveDevice(bool VALUE); //DAQinstruction.cpp

  void system_card_trig_valid(bool valid); //DAQinstruction.cpp

  void system_slave_card_trig_valid(bool valid); //DAQinstruction.cpp

  void hard_reset(bool DEVICE = false); //DAQinstruction.cpp

  void usb_force_wakeup(); //DAQinstruction.cpp

  void readACDC_RAM(int device, unsigned int mask); //DAQinstruction.cpp

  void cleanup(); //SuMo.cpp

  void prep_sync(); //DAQinstruction.cpp

  void make_sync(); //DAQinstruction.cpp

  //send trigger over software
  void software_trigger(unsigned int SOFT_TRIG_MASK, bool set_bin = false, unsigned int bin = 0); //DAQinstruction.cpp

  void software_trigger_slaveDevice(unsigned int SOFT_TRIG_MASK, bool set_bin = false, unsigned int bin = 0); //DAQinstruction.cpp

  //readout functions
  int check_readout_mode(); //SuMo.cpp

  int read_CC(bool SHOW_CC_STATUS, bool SHOW_AC_STATUS, int device = 0, int triGmode = 0); //GetSysPackets.cpp

  int measure_rate(bool *AC_read_mask); //SuMo.cpp //To be used in Automation function.
  void adjust_thresh(int threshold, unsigned int board_number); //SuMo.cpp

  /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for additional retval handling: */
  int read_AC(unsigned int trig_mode, bool *mask, bool FILESAVE,
              bool sync = false, bool set_bin = false, unsigned int bin = 0); //GetAcdcPackets.cpp

  void dump_data(); //SuMo.cpp

  //int  get_AC_info(bool PRINT, int AC_adr);
  int *get_AC_info(bool PRINT, int frontEnd, bool PRINTALL = false, int count = 0, double time = 0.,
                   double dateTime = 0., int evts = 0);   //form_meta_data.cpp //parse meta-data from raw data stream

  int generate_ped(
          bool ENABLE_FILESAVE);  //makePedandLin.cpp            //generate pedestal calibration files for each active board (one per sample cell)
  int make_count_to_voltage();  //makePedandLin.cpp         //make count-to-voltage LUT for each active board (# active boards * 6 channels * 1536 cells * 4096 !!)

  int load_ped(); //loadPedandLin.cpp

  int log_data(unsigned int NUM_READS, int trig_mode, int acq_rate, const char *log_filename); //dataIO.cpp


  int check_active_boards(bool print); //SuMo.cpp

  int check_active_boards(int NUM); //SuMo.cpp

  int check_active_boards_slaveDevice(); //SuMo.cpp

  bool DC_ACTIVE[numFrontBoards];           //TRUE if boards are connected and synced
  bool EVENT_FLAG[numFrontBoards];
  bool CAUGHT_EVENT_FLAG[numFrontBoards];
  bool DIGITIZING_START_FLAG[numFrontBoards];
  bool BOARDS_READOUT[numFrontBoards];      //TRUE if board was successfully readout when data requested
  bool BOARDS_TIMEOUT[numFrontBoards];      //TRUE if board was unsuccessfully readout when data requested (timeout error)

  unsigned int CC_EVENT_COUNT_FROMCC0;
  unsigned int CC_EVENT_COUNT_FROMCC1;

  struct packet_t *adcDat[numFrontBoards];
  enum cc_readout_mode mode;

  static void sys_wait(unsigned int sleep) {
      usleep(sleep);
  }

  bool fileExists(const std::string &filename); //SuMo.cpp

  unsigned int PED_DATA[numFrontBoards][AC_CHANNELS][psecSampleCells];

private:
  void createUSBHandles(); //DAQinstruction.cpp

  void closeUSBHandles(); //DAQinstruction.cpp

  int unwrap(int ASIC); //SuMo.cpp

  void unwrap_baseline(int *baseline, int ASIC); //SuMo.cpp

  void print_to_terminal(int k, int NUM_READS, int cc_event, int board_trig, double t); //dataIO.cpp

  struct cal_t {
    unsigned short raw_ped_data_array[AC_CHANNELS][psecSampleCells][num_ped_reads];
    /* calibration arrays */
    unsigned int PED_DATA[AC_CHANNELS][psecSampleCells];
    float PED_RMS[AC_CHANNELS][psecSampleCells];
    bool PED_SUCCESS;
  } calData[numFrontBoards];

  static int compare(const void *a, const void *b) {
      return (*(unsigned short *) a - *(unsigned short *) b);
  }

  int metaData[psecSampleCells];

  int put_lut_on_heap(
          bool *range); //makePedandLin.cpp // cpu memory management when taking full count-to-voltage calibration scan
  void free_lut_from_heap(
          bool *range); //makePedandLin.cpp // cpu memory management when taking full count-to-voltage calibration scan

  float ***LUT_CELL;
  int ***LUT_CELL_COPY;
  float ***LUT;
  float ***LUT_COPY;

  /* metadata from CC */
  unsigned int CC_INFO[cc_buffersize];
  unsigned int LAST_CC_INSTRUCT;
  unsigned int CC_TIMESTAMP_LO;
  unsigned int CC_TIMESTAMP_HI;
  unsigned int CC_EVENT_COUNT;
  unsigned int CC_BIN_COUNT;
  unsigned int CC_EVENT_NO;

protected:
  stdUSB usb;        //object to handle USB device
  stdUSBSlave usb2;       //object to handle USB slave device

};
