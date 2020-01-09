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

            cout << "samples received: " << samples << " on board " << boardAddress << endl;

            //if the number of samples received is negative, 
            //that means the retval in readData was -1 and the
            //read failed. 
            if(samples < 0)
            {
                cout << "No data were in the usb buffer on read mode board address: " << boardAddress << endl;
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

            /* packet flags */
           
            int data_header = 0;
            int data_adc_footer = 0;
            int data_footer = 0;
            int usb_read_offset_flag = -1;

            /* check data in packet */
            int checkpkt = 0;
            while (checkpkt < cc_buffersize) {
                adcDat[boardAddress]->CC_HEADER_INFO[checkpkt] = buffer[checkpkt];
                if (print) cout << checkpkt << ":" << buffer[checkpkt] << "  ";

                if (buffer[checkpkt] == dataPacketStart) {
                    usb_read_offset_flag = checkpkt;
                    break;
                }
                checkpkt++;
            }

            if (usb_read_offset_flag < 0) {
                BOARDS_TIMEOUT[boardAddress] = true;
                if (print) cout << "timeout on board " << boardAddress << endl;
                continue;
            }

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

            CC_BIN_COUNT = (buffer[1] & 0x18) >> 3;
            CC_EVENT_NO = buffer[2];

            if (print) {
                cout << buffer[0] << "," << buffer[1] << "," << buffer[2] << ","
                     << buffer[3] << "," << buffer[4] << "," << buffer[5] << "," << endl;
                cout << "packet header index " << adcDat[boardAddress]->PKT_HEADER << endl;
                cout << "packet data indices " << adcDat[boardAddress]->DATA_HEADER[0] << ","
                     << adcDat[boardAddress]->DATA_ADC_END[0] << ","
                     << adcDat[boardAddress]->DATA_FOOTER[0] << " "
                     << adcDat[boardAddress]->DATA_HEADER[1] << "," << adcDat[boardAddress]->DATA_ADC_END[1] << ","
                     << adcDat[boardAddress]->DATA_FOOTER[1] << " "
                     << adcDat[boardAddress]->DATA_HEADER[2] << "," << adcDat[boardAddress]->DATA_ADC_END[2] << ","
                     << adcDat[boardAddress]->DATA_FOOTER[2] << " "
                     << adcDat[boardAddress]->DATA_HEADER[3] << "," << adcDat[boardAddress]->DATA_ADC_END[3] << ","
                     << adcDat[boardAddress]->DATA_FOOTER[3] << " "
                     << adcDat[boardAddress]->DATA_HEADER[4] << "," << adcDat[boardAddress]->DATA_ADC_END[4] << ","
                     << adcDat[boardAddress]->DATA_FOOTER[4] << endl;
                cout << "packet footer index " << adcDat[boardAddress]->PKT_FOOTER << endl;
            }
            /* real data starts here: */
            /* form usable data from packets */

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
