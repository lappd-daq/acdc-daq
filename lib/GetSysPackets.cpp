#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <math.h>
#include <sstream>
#include <bitset>
#include "SuMo.h"

using namespace std;

int SuMo::read_CC(bool SHOW_CC_STATUS, bool SHOW_AC_STATUS, int device, int triggerMode) {

    bool print = false; // verbose
    int samples;        // no. of words in usb packet
    int samples_device_0, samples_device_1;

    if (device == 0)
        for (int i = 0; i < 4; i++) {
            EVENT_FLAG[i] = false;
            DIGITIZING_START_FLAG[i] = false;
        }
    else if (device == 1)
        for (int i = 4; i < numFrontBoards; i++) {
            EVENT_FLAG[i] = false;
            DIGITIZING_START_FLAG[i] = false;
        }
    else if (device == 100) {  //device = 100 for all boards in system
        for (int i = 0; i < numFrontBoards; i++) {
            EVENT_FLAG[i] = false;
            DIGITIZING_START_FLAG[i] = false;
        }
        if (print) cout << "using device = 100, ALL devices " << endl;
    } else return -3; // device assignment error

    /* device = 1 for slave device */
    if (device > 1 && mode != USB2x) return -2;

    /* set usb read mode for numFrontBoards+1 (central card readout-only) */
    if (device == 0) set_usb_read_mode(5);
    else if (device == 1) set_usb_read_mode_slaveDevice(5);
    else if (device == 100) {
        if (SHOW_CC_STATUS) cout << "master CC connected." << endl;
        if (mode == USB2x) {
            set_usb_read_mode_slaveDevice(5);
            if (SHOW_CC_STATUS) cout << "slave CC connected." << endl;
        }
        set_usb_read_mode(5);
    } else return -3;

    int ram_events = 0;
    int num_devices = 1;
    int device_tmp = device;
    if (device == 100) {
        num_devices = 2;
        device_tmp = 0;
    } else {
        num_devices = 1;
        device_tmp = device;
    }

    for (int loop_cc = device_tmp; loop_cc < device_tmp + num_devices; loop_cc++) {

        if (print) cout << "device in central card address loop : " << loop_cc << endl;

        unsigned short buffer[cc_buffersize];
        memset(buffer, 0x0, cc_buffersize * sizeof(unsigned short));
        /* try readout */
        try {
            if (loop_cc == 0) {
                usb.readData(buffer, cc_buffersize + 2, &samples);
                samples_device_0 = samples;
            } else if (loop_cc == 1) {
                usb2.readData(buffer, cc_buffersize + 2, &samples);
                samples_device_1 = samples;
            }

            if (samples < 2 && device != 100) {
                if (print) cout << "error: no data in buffer on device #" << device << endl;
                return -1;
            }

            int cc_header_found = -1;
            int cc_start_found = -1;

            //debugging, save entire buffer 
            /*
            ofstream ACC_bufferfile;
            char ACC_bufferfilename[200];
            sprintf(ACC_bufferfilename, "acc-buffer-top-%i.txt", std::rand() % 100);
            ACC_bufferfile.open(ACC_bufferfilename, ios::trunc);
            cout << "Opened file : " << ACC_bufferfilename << " to dump the buffer from the ACC" << endl;
            ACC_bufferfile << "Decimal, hex, binary" << endl;
            
            for(int i = 0; i < cc_buffersize + 2; i++)
            {
                ACC_bufferfile << buffer[i] << ", ";
                stringstream ss;
                ss << std::hex << buffer[i];
                string hexstr(ss.str());
                ACC_bufferfile << hexstr << ", ";
                unsigned n;
                ss >> n;
                bitset<16> b(n);
                ACC_bufferfile << b.to_string() << endl;
            }
            ACC_bufferfile.close();
            */
            //end debugging printing
        


            for (int i = 0; i < 5; i++) {
                if (buffer[i] == 0x1234) {
                    cc_header_found = i;
                    if (print) cout << "cc header @ " << i << endl;
                }
                if (buffer[i] == 0xDEAD) {
                    cc_start_found = i;
                    if (print) cout << "cc start @ " << i << endl;
                }
            }
            if (loop_cc == 0) {
                CC_EVENT_COUNT_FROMCC0 = buffer[5];
                if (SHOW_CC_STATUS) cout << "master event = " << buffer[5] << endl;
            }
            if (loop_cc == 1) {
                CC_EVENT_COUNT_FROMCC1 = buffer[5];
                if (SHOW_CC_STATUS) cout << "slave event = " << buffer[5] << endl;
            }

            for (int i = 0; i < cc_buffersize; i++) {
                CC_INFO[i] = buffer[i];
                if (print) cout << i << ":" << buffer[i] << " ";
            }
            if (print) cout << "samples received: " << samples << " on device #" << device << endl;
        }
        catch (...) {
            fprintf(stderr, "Please connect the board. [DEFAULT exception] \n");
            return 1;
        }

        if (SHOW_CC_STATUS) {
            cout << endl;
            if (device > 0) cout << "AC/DC connection status :: Slave Device: \n";
            else cout << "AC/DC connection status: \n";
        }

        // Initialize DC_ACTIVE to false
        for (int ac = 0; ac < numFrontBoards; ac++) {
            DC_ACTIVE[ac] = false;
        }

        int slave_index = loop_cc * boardsPerCC;
        /* look for ACDC boards, set global variable DC_ACTIVE[numFrontBoards] */
        // add second handle for DC_ACTIVE
        if ((buffer[2] & 0x1) && (buffer[2] & 0x11)) {
            DC_ACTIVE[0 + slave_index] = true;
            if (SHOW_CC_STATUS) cout << "* DC 0 detected!! \n";
        }
        if ((buffer[2] & 0x2) && (buffer[2] & 0x22)) {
            DC_ACTIVE[1 + slave_index] = true;
            if (SHOW_CC_STATUS) cout << "* DC 1 detected!! \n";
        }
        if ((buffer[2] & 0x4) && (buffer[2] & 0x44)) {
            DC_ACTIVE[2 + slave_index] = true;
            if (SHOW_CC_STATUS) cout << "* DC 2 detected!! \n";
        }
        if ((buffer[2] & 0x8) && (buffer[2] & 0x88)) {
            DC_ACTIVE[3 + slave_index] = true;
            if (SHOW_CC_STATUS) cout << "* DC 3 detected!! \n";
        }

        // TODO the above will have to be extended when switching to ACC


        //check for events in CC RAM
        if (buffer[4] & 0x1) EVENT_FLAG[0 + slave_index] = true;
        if (buffer[4] & 0x2) EVENT_FLAG[1 + slave_index] = true;
        if (buffer[4] & 0x4) EVENT_FLAG[2 + slave_index] = true;
        if (buffer[4] & 0x8) EVENT_FLAG[3 + slave_index] = true;

        ram_events += EVENT_FLAG[0 + slave_index] +
                EVENT_FLAG[1 + slave_index] +
                EVENT_FLAG[2 + slave_index] +
                EVENT_FLAG[3 + slave_index];

        if (buffer[4] >> 4 & 0x1) DIGITIZING_START_FLAG[0 + slave_index] = true;
        if (buffer[4] >> 4 & 0x2) DIGITIZING_START_FLAG[1 + slave_index] = true;
        if (buffer[4] >> 4 & 0x4) DIGITIZING_START_FLAG[2 + slave_index] = true;
        if (buffer[4] >> 4 & 0x8) DIGITIZING_START_FLAG[3 + slave_index] = true;

    }  //end loop over central cards
//probe ac/dc cards if specified
    if (SHOW_AC_STATUS) {
        bool tmp_active[numFrontBoards];
        for (int ii = 0; ii < numFrontBoards; ii++) tmp_active[ii] = false; //mask off master or slave board for printing info

        if (device == 1) {     //slave device
            for (int board = boardsPerCC; board < numFrontBoards; board++) tmp_active[board] = DC_ACTIVE[board];

            read_AC(triggerMode, tmp_active, false);
            for (int board = boardsPerCC; board < numFrontBoards; board++)
                if (DC_ACTIVE[board]) {
                    cout << endl << "AC/DC #" << board << ":";
                    get_AC_info(true, board);
                }
        } else if (device == 0) {          //master device
            for (int board = 0; board < boardsPerCC; board++) tmp_active[board] = DC_ACTIVE[board];


            read_AC(triggerMode, tmp_active, false);
            for (int board = 0; board < boardsPerCC; board++) {
                if (DC_ACTIVE[board]) {
                    cout << endl << "AC/DC #" << board << ":";
                    get_AC_info(true, board);
                }
            }

        } else if (device == 100) {          //all devices

            read_AC(triggerMode, DC_ACTIVE, false, true);
            for (int board = 0; board < numFrontBoards; board++)
                if (DC_ACTIVE[board]) {
                    cout << endl << "AC/DC #" << board << ":";
                    get_AC_info(true, board);
                }
        }
        sys_wait(5000);
        manage_cc_fifo(1);
        if (mode == USB2x) manage_cc_fifo_slaveDevice(1);
    }

    if (SHOW_CC_STATUS) cout << "\n**********************\n";

    //if(device)  usb2.freeHandles();
    //else        usb.freeHandles();

    if (print) cout << "retval, if success is ram_events = " << ram_events << endl;
    return ram_events;
}
