#pragma once

#define numFrontBoards        8        // max number of plugged boards
#define boardsPerCC           8        // max number of AC/DC cards per central card
#define cc_buffersize         31       // Central-card meta-data size
#define ac_buffersize         8100     // firmware packet size should not exceed this
#define AC_CHANNELS           30       // number of channels per AC/DC board
#define psec_buffersize       1536     // data size per ASIC
#define infoBuffersize        17       // meta-data size per ASIC
#define USB_READ_OFFSET       13       // start of data in packet
#define psecSampleCells       256      // length of PSEC4 waveform in sample steps
#define numChipsOnBoard       5        // number of PSEC4's on each AC/DC boards
#define num_ped_reads         50       // number of pedestal readouts to generate ped.

#define WRAP_CONSTANT         230      // constant for PSEC4 wraparound correction
#define USE_ACC               true
