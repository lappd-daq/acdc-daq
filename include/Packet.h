#pragma once

#include "Definitions.h"

/* waveform and meta-data arrays from ADC boards */
struct packet_t {
    unsigned int last_ac_instruct;
    unsigned int event_count;
    unsigned int dig_event_count;
    unsigned int timestamp_hi; /* 16 bits */
    unsigned int timestamp_mid;/* 16 bits */
    unsigned int timestamp_lo; /* 16 bits */
    unsigned int dig_timestamp_hi; /* 16 bits */
    unsigned int dig_timestamp_mid;/* 16 bits */
    unsigned int dig_timestamp_lo; /* 16 bits */
    unsigned int vbias[numChipsOnBoard];
    unsigned int ro_cnt[numChipsOnBoard];
    unsigned int ro_target_cnt[numChipsOnBoard];
    unsigned int ro_dac_value[numChipsOnBoard];
    int last_sampling_bin[numChipsOnBoard];
    unsigned int trigger_threshold[numChipsOnBoard];
    // set by trig_settings
    bool trig_sign;
    bool trig_rate_only;
    bool trig_wait_for_sys;
    bool trig_en;
    bool use_sma_trig_input;
    bool use_coincidence_settings;
    bool use_trig_valid_as_reset;
    unsigned int coincidence_window;
    // set by trig_settings_2
    unsigned int sys_coincidence_width;
    unsigned int coincidence_num_chips;
    unsigned int coincidence_num_chans;
    unsigned int last_coincidence_num_chans;

    unsigned int bin_count_rise;
    unsigned int self_trig_settings;
    unsigned int self_trig_settings_2;
    unsigned int reg_self_trig;
    unsigned int self_trig_mask;
    unsigned int self_trig_scalar[AC_CHANNELS];

    unsigned int counts_of_sys_no_local;
    unsigned int sys_trig_count;
    unsigned int resets_from_firmw;
    unsigned int time_from_valid_to_trig;
    unsigned int firmware_reset_time;

    unsigned int firmware_version;

    unsigned int CC_HEADER_INFO[cc_buffersize];
    unsigned int CC_TIMESTAMP_LO;
    unsigned int CC_TIMESTAMP_MID;
    unsigned int CC_TIMESTAMP_HI;
    unsigned int CC_EVENT_COUNT;
    unsigned int CC_BIN_COUNT;
    bool CC_TRIG_MODE;

    unsigned int PKT_HEADER;
    unsigned int PKT_FOOTER;

    unsigned int DATA_HEADER[numChipsOnBoard];
    unsigned int DATA_ADC_END[numChipsOnBoard];
    unsigned int DATA_FOOTER[numChipsOnBoard];
    /* raw data saved here */
    unsigned short AC_RAW_DATA[numChipsOnBoard][psec_buffersize];
    unsigned int Data[AC_CHANNELS][psecSampleCells]; /* AC_CHANNELS waveforms + 1 metadata */
    unsigned int Meta[psecSampleCells];
    unsigned short AC_INFO[numChipsOnBoard][infoBuffersize];
};
