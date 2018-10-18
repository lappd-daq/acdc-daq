#pragma once

#define numFrontBoards        8        //max number of plugged boards
#define boardsPerCC           8        //max number of AC/DC cards per central card
#define cc_buffersize         31       //Central-card meta-data size
#define ac_buffersize         8100     //firmware packet size should not exceed this
#define AC_CHANNELS           30       //number of channels per AC/DC board
#define psec_buffersize       1536     //data size per ASIC
#define infoBuffersize        17       //meta-data size per ASIC
#define USB_READ_OFFSET       13       //start of data in packet
#define psecSampleCells       256      //length of PSEC4 waveform in sample steps
#define numChipsOnBoard       5        //number of PSEC4's on each AC/DC boards
#define num_ped_reads         50       //number of pedestal readouts to generate ped.

#define WRAP_CONSTANT         230      //constant for PSEC4 wraparound correction

#define usbPacketStart        0x1234   //packet header word
#define usbPacketEnd          0x4321   //packet footer word
#define dataPacketStart       0xF005   //packet data start
#define adcPacketEnd          0xBA11   //packet end-of-waveform
#define dataPacketEnd         0xFACE   //packet end-of-AC/DC-data

#define VID1                  0x6672
#define VID2                  0x6672
#define PID1                  0x2920
#define PID2                  0x2921
