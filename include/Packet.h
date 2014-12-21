#ifndef __PACKET_H__
#define __PACKET_H__

#include "Definitions.h"

/* waveform and meta-data arrays from ADC boards */
struct packet_t{
  unsigned int           last_ac_instruct;
  unsigned int           last_last_ac_instruct;
  unsigned int           event_count;
  unsigned int           timestamp_hi; /* 16 bits */
  unsigned int           timestamp_mid;/* 16 bits */
  unsigned int           timestamp_lo; /* 16 bits */
  int                    vbias[numChipsOnBoard];
  float                  ro_cnt[numChipsOnBoard];
  float                  ro_target_cnt[numChipsOnBoard];
  float                  ro_dac_value[numChipsOnBoard];
  int                    last_sampling_bin[numChipsOnBoard];
  float                  trigger_threshold[numChipsOnBoard];
  bool                   trig_sign;
  bool                   trig_rate_only;
  bool                   trig_wait_for_sys;
  bool                   trig_en;
  unsigned int           bin_count_rise;
  unsigned int           bin_count_fall;
  unsigned int           self_trig_settings;
  unsigned int           reg_self_trig[4];
  unsigned int           self_trig_mask;
  unsigned int           self_trig_scalar[AC_CHANNELS];  
  
  unsigned int           CC_HEADER_INFO[cc_buffersize];
  unsigned int           CC_TIMESTAMP_LO;
  unsigned int           CC_TIMESTAMP_MID;
  unsigned int           CC_TIMESTAMP_HI;
  unsigned int           CC_EVENT_COUNT;
  unsigned int           CC_BIN_COUNT;
  bool                   CC_TRIG_MODE;
  
  unsigned int           PKT_HEADER;
  unsigned int           PKT_FOOTER ;
  
  unsigned int           DATA_HEADER[numChipsOnBoard];
  unsigned int           DATA_ADC_END[numChipsOnBoard];
  unsigned int           DATA_FOOTER[numChipsOnBoard];
  /* raw data saved here */
  unsigned short         AC_RAW_DATA[numChipsOnBoard][psec_buffersize];
  float                  Data[AC_CHANNELS+1][psecSampleCells]; /* AC_CHANNELS waveforms + 1 metadata */
  unsigned short         AC_INFO[numChipsOnBoard][infoBuffersize];
};

#endif
