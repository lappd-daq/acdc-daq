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
  SuMo();

  ~SuMo();

  void sync_usb(bool SYNC);

  //instructions to hardware
  void reset_dll(bool sync = false);

  void reset_self_trigger(unsigned int mask, int device = 0);

  void reset_time_stamp(bool sync = false);

  void reset_acdc();

  void align_lvds();

  //front-end card specific instructions
  void toggle_LED(bool EN);

  void toggle_CAL(bool EN, int device = 0);

  void set_self_trigger_lo(bool ENABLE_TRIG, bool SYS_TRIG_OPTION,
                           bool RATE_ONLY, bool TRIG_SIGN, bool USE_SMA,
                           bool USE_COINCIDENCE, bool USE_FAST_COINCIDENCE,
                           unsigned int coinc_window,
                           unsigned int mask, int device = 0);

  void set_self_trigger_hi(unsigned int coinc_pulse_width,
                           unsigned int asic_coincidence_min,
                           unsigned int channel_coincidence_min,
                           unsigned int mask, int device = 0);

  void set_self_trigger_mask(int mask, bool HILO, unsigned int board_mask, int device = 0);

  //chip-level instructions
  void set_pedestal_value(unsigned int PED_VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31);

  void set_dll_vdd(unsigned int VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31);

  void set_trig_threshold(unsigned int TRIG_VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31);

  void
  set_ro_target_count(unsigned int TARGET_RO_COUNT, unsigned int mask, int device = 0, unsigned int psec_mask = 31);

  //manage PC interface
  void set_usb_read_mode(unsigned int READ_MODE);

  void set_usb_read_mode_slaveDevice(unsigned int READ_MODE);

  void manage_cc_fifo(bool VALUE);

  void manage_cc_fifo_slaveDevice(bool VALUE);

  void system_card_trig_valid(bool valid);

  void system_slave_card_trig_valid(bool valid);

  void hard_reset(bool DEVICE = false);

  void usb_force_wakeup();

  void readACDC_RAM(int device, unsigned int mask);

  void cleanup();

  void prep_sync();

  void make_sync();

  //send trigger over software
  void software_trigger(unsigned int SOFT_TRIG_MASK, bool set_bin = false, unsigned int bin = 0);

  void software_trigger_slaveDevice(unsigned int SOFT_TRIG_MASK, bool set_bin = false, unsigned int bin = 0);

  //readout functions
  int check_readout_mode();

  int read_CC(bool SHOW_CC_STATUS, bool SHOW_AC_STATUS, int device = 0, int triGmode = 0);

  int measure_rate(bool *AC_read_mask); //To be used in Automation function.
  void adjust_thresh(int threshold, unsigned int board_number);

  /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for additional retval handling: */
  int read_AC(unsigned int trig_mode, bool *mask, bool FILESAVE,
              bool sync = false, bool set_bin = false, unsigned int bin = 0);

  void dump_data();

  //int  get_AC_info(bool PRINT, int AC_adr);
  int *get_AC_info(bool PRINT, int frontEnd, bool PRINTALL = false, int count = 0, double time = 0.,
                   double dateTime = 0., int evts = 0);         //parse meta-data from raw data stream

  int generate_ped(
          bool ENABLE_FILESAVE);              //generate pedestal calibration files for each active board (one per sample cell)
  int
  make_count_to_voltage();                     //make count-to-voltage LUT for each active board (# active boards * 6 channels * 1536 cells * 4096 !!)

  int load_ped();

  int log_data(unsigned int NUM_READS, int trig_mode, int acq_rate, const char *log_filename);


  int check_active_boards(bool print);

  int check_active_boards(int NUM);

  int check_active_boards_slaveDevice();

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

  bool fileExists(const std::string &filename);

  unsigned int PED_DATA[numFrontBoards][AC_CHANNELS][psecSampleCells];

private:
  void createUSBHandles();

  void closeUSBHandles();

  int unwrap(int ASIC);

  void unwrap_baseline(int *baseline, int ASIC);

  void print_to_terminal(int k, int NUM_READS, int cc_event, int board_trig, double t);

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
          bool *range); // cpu memory management when taking full count-to-voltage calibration scan
  void free_lut_from_heap(
          bool *range); // cpu memory management when taking full count-to-voltage calibration scan

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
