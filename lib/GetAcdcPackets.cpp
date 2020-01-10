#include "SuMo.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <bitset>

using namespace std;

int SuMo::read_AC(unsigned int trig_mode, bool *mask,
                  bool sync, bool set_bin, unsigned int bin) {
    //cout << "read_AC!!!" << endl;
    bool print = false;     /* verbose switch for dumping info to terminal */
    int numBoardsRead = 0; /* function returns number of front end cards successfully readout (int) */

    /* send trigger over USB from this function, if specified */
    unsigned int trig_mask = 0;
    unsigned int trig_mask_slave = 0;

    //loop over master board
    for (int boardAddress = 0; boardAddress < boardsPerCC; boardAddress++)
        trig_mask = (mask[boardAddress] << boardAddress) | trig_mask;
    if (print) cout << "trig mask " << trig_mask << endl;

    //temporarily loop over slave board, eventually connected in firmware
    for (int boardAddress = boardsPerCC; boardAddress < numFrontBoards; boardAddress++)
        trig_mask_slave = (mask[boardAddress] << (boardAddress - 4)) | trig_mask_slave;
    if (print) cout << "trig mask on slave " << trig_mask_slave << endl;

    if (trig_mode == 0) {
        if (sync) {
            prep_sync();
            software_trigger(trig_mask, set_bin, bin);
            software_trigger_slaveDevice(trig_mask_slave, set_bin, bin);
            make_sync();
        } else {
            software_trigger(trig_mask, set_bin, bin);
            software_trigger_slaveDevice(trig_mask_slave, set_bin, bin);
        }
    } else if (trig_mode == 99) {
        readACDC_RAM(0, trig_mask);
        if (mode == USB2x) readACDC_RAM(1, trig_mask_slave);
    }

    /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for retval handling*/

    for (int boardAddress = 0; boardAddress < numFrontBoards; boardAddress++) {
        BOARDS_READOUT[boardAddress] = false;
        BOARDS_TIMEOUT[boardAddress] = false;
        if (DC_ACTIVE[boardAddress] == false || mask[boardAddress] == false) {
            continue;
        }
        int device = 0;                                 //default 'master' device
        if (boardAddress >= boardsPerCC && mode == USB2x)  //specify slave device if conditions satisfied
            device = 1;

        if (print) cout << "reading board: " << boardAddress << endl;

        usleep(100);
        if (device == 1) set_usb_read_mode_slaveDevice(boardAddress + 1 - boardsPerCC);
        else set_usb_read_mode(boardAddress + 1);


        /* try to get packet from addressed AC/DC card */
        try {
            int samples;
            unsigned short* buffer;
            buffer = (unsigned short*)calloc(ac_buffersize + 2, sizeof(unsigned short));
            if (device == 1) usb2.readData(buffer, ac_buffersize + 2, &samples);
            else usb.readData(buffer, ac_buffersize + 2, &samples);


            //if the number of samples received is negative, 
            //that means the retval in readData was -1 and the
            //read failed. 
            if(samples < 0)
            {
                cout << "No data were in the usb buffer on read mode board address: " << boardAddress << endl;
                throw std::exception();
            }


            //debugging, save entire buffer 
            /*
            ofstream ACDC_bufferfile;
            char ACDC_bufferfilename[200];
            sprintf(ACDC_bufferfilename, "acdc-buffer-%i.txt", std::rand() % 100);
            ACDC_bufferfile.open(ACDC_bufferfilename, ios::trunc);
            cout << "Opened file : " << ACDC_bufferfilename << " to dump the buffer from the ACDC" << endl;
            ACDC_bufferfile << "Decimal, hex, binary" << endl;
            
            for(int i = 0; i < ac_buffersize + 2; i++)
            {
                ACDC_bufferfile << buffer[i] << ", ";
                stringstream ss;
                ss << std::hex << buffer[i];
                string hexstr(ss.str());
                ACDC_bufferfile << hexstr << ", ";
                unsigned n;
                ss >> n;
                bitset<16> b(n);
                ACDC_bufferfile << b.to_string() << endl;
            }
            ACDC_bufferfile.close();
            */
            //end debugging printing
     

            //save CC meta-data at the front of every buffer
            int save_byte_count = 0;
            int checkpkt = 0;
            bool start_found = false;
            //save a constant number of bytes after 1234, set in definitions as ccinfoBuffersize
            while(save_byte_count < ccinfoBuffersize && checkpkt < cc_buffersize+2) {
                //have not found start byte 1234 yet
                if(start_found == false)
                {
                    if(buffer[checkpkt] == usbPacketStart)
                    {
                        start_found = true;
                    }
                }
                else
                {
                    adcDat[boardAddress]->CC_HEADER_INFO[save_byte_count] = buffer[checkpkt];
                    save_byte_count++;
                    //cout << "Adding byte to cc-header-info["<<save_byte_count<<"] = " << buffer[checkpkt] << endl;
                }
                checkpkt++;
            }



            int data_header = 0;
            int data_adc_footer = 0;
            int data_footer = 0;
            /* interpret data packet, save raw data to file if enabled */
            for (int i = 0; i < ac_buffersize; i++) {
                if (buffer[i] == usbPacketStart) adcDat[boardAddress]->PKT_HEADER = i;
                else if (buffer[i] == usbPacketEnd) {
                    adcDat[boardAddress]->PKT_FOOTER = i;
                    break;
                } else if (buffer[i] == dataPacketStart) {
                    adcDat[boardAddress]->DATA_HEADER[data_header] = i;
                    data_header++;
                } else if (buffer[i] == adcPacketEnd) {
                    adcDat[boardAddress]->DATA_ADC_END[data_adc_footer] = i;
                    data_adc_footer++;
                } else if (buffer[i] == dataPacketEnd) {
                    adcDat[boardAddress]->DATA_FOOTER[data_footer] = i;
                    data_footer++;
                }
            }

            //don't know what the &0x18 is about or the 3 bit shift. 
            CC_BIN_COUNT = (adcDat[boardAddress]->CC_HEADER_INFO[0] & 0x18) >> 3;
            //evt number in CC is two 16 bit counters, with most significant coming first
            unsigned int evt_no_upper = adcDat[boardAddress]->CC_HEADER_INFO[1];
            unsigned int evt_no_lower = adcDat[boardAddress]->CC_HEADER_INFO[2];
            CC_EVENT_NO = (evt_no_upper << 16) | evt_no_lower;



            //waveform data parsing here
            for (int i = 0; i < numChipsOnBoard; i++) {
                //get raw data from buffer
                for (int j = 0; j < psec_buffersize; j++) {
                    adcDat[boardAddress]->AC_RAW_DATA[i][j] = buffer[adcDat[boardAddress]->DATA_HEADER[i] + j + 1];    
                }
                //get metadata (like displayed on ./bin/readACDC) from buffer
                for (int k = 0; k < infoBuffersize; k++) {
                    adcDat[boardAddress]->AC_INFO[i][k] = buffer[adcDat[boardAddress]->DATA_ADC_END[i] + k + 1];
                }
            }

            //grab data on scalar info after all chip data
            for (int i = 0; i < AC_CHANNELS + 1; i++) {
                adcDat[boardAddress]->self_trig_scalar[i] = buffer[adcDat[boardAddress]->DATA_FOOTER[numChipsOnBoard - 1] + 2 + i];
                if (print) cout << i << endl;
                if (print) cout << adcDat[boardAddress]->self_trig_scalar[i] << endl;
                //	cout << adcDat[boardAddress]->DATA_FOOTER[numChipsOnBoard-1]+2+i << " " << adcDat[boardAddress]->self_trig_scalar[i] << endl;
            }
            //      for(int p=7790; p < ac_buffersize; p++){

            //       cout << buffer[p] << " " << p <<  endl;
            // }
            // for(int i=-100; i<100; i++){
            //	        adcDat[boardAddress]->self_trig_scalar[i] = buffer[adcDat[boardAddress]->DATA_FOOTER[numChipsOnBoard-1]+2+i];
            //		cout << adcDat[boardAddress]->self_trig_scalar[i] << endl;
            //       }
            //wrong index for self_trig_scalar?
            BOARDS_READOUT[boardAddress] = true;
            numBoardsRead++;
            //cout << numBoardsRead << endl;
            free(buffer); //release this allocated memory
        }

        catch (...) {
            fprintf(stderr, "Please connect the board. [DEFAULT exception]\n");
            return 1;
        }

        

    }
    return numBoardsRead;

}
