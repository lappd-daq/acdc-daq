#include "config.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>

using namespace std;

// configuration variables:
unsigned int trig_mask[numFrontBoards];
bool trig_enable[numFrontBoards];
unsigned int pedestal[numFrontBoards][numChipsOnBoard];
unsigned int threshold[numFrontBoards][numChipsOnBoard];
bool trig_sign[numFrontBoards];
bool wait_for_sys;
bool rate_only;
bool sma_trig_on_fe[numFrontBoards];
bool hrdw_trig;
bool hrdw_trig_sl;
unsigned int hrdw_trigsrc;
unsigned int hrdw_trig_slsrc;

// programmability for 'wait_for_sys' self-trig coincidence mode
unsigned int coinc_window;
unsigned int coinc_pulsew;
bool use_coinc;
bool use_trig_valid;
unsigned int coinc_num_ch;
unsigned int coinc_num_asic;

// default trig settings (OFF)
void set_default_values() {
    for (int i = 0; i < numFrontBoards; i++) {
        for (int j = 0; j < numChipsOnBoard; j++) {
            pedestal[i][j] = 0x800;
            threshold[i][j] = 0x000;
        }

        trig_mask[i] = 0x00000000;  // 32 bit
        trig_enable[i] = false;
        sma_trig_on_fe[i] = false;
        trig_sign[i] = 0;

    }

    wait_for_sys = false;
    rate_only = false;
    hrdw_trig = false;
    hrdw_trig_sl = false;
    hrdw_trigsrc = 0;
    hrdw_trig_slsrc = 0;
    use_coinc = false;
    use_trig_valid = false;
    coinc_num_ch = 0;
    coinc_num_asic = 0;
    cout << "self-trigger disabled" << endl;
}

// write registers
int write_config_to_hardware(SuMo &Sumo, bool WRITETRIG, bool WRITEACDC) {
    int device = 0;

    cout << "__________________________" << endl;

    Sumo.set_usb_read_mode(16);

    int mode = Sumo.check_readout_mode();
    if (mode == 1 && Sumo.check_active_boards_slaveDevice() > 0) {
        Sumo.set_usb_read_mode_slaveDevice(16);
    }

    Sumo.dump_data();

    for (int jj = 0; jj < 2; jj++) {
        for (int i = 0; i < numFrontBoards; i++) {
            //Sumo.reset_self_trigger();
            unsigned int boardAddress = pow(2, i % 4);
            if (i >= 4) device = 1;

            if (jj == 1) {
                if (Sumo.DC_ACTIVE[i] == false) {
                    cout << "no board = " << i << " addressed at device "
                         << device << ":0x" << hex << boardAddress << endl;
                    //cout << "__________________________" << std::dec << endl;
                    continue;
                }

                cout << "writing settings to board " << i << " addressed at device "
                     << device << ":0x" << hex << boardAddress << endl;
            }

            if (WRITETRIG) {
                /* send trig mask to boards in 2 sets of 16 bit words */
                Sumo.set_self_trigger_mask(0x00007FFF & trig_mask[i], 0, boardAddress, device);
                Sumo.set_self_trigger_mask((0x3FFF8000 & trig_mask[i]) >> 15, 1, boardAddress, device);


                Sumo.set_self_trigger_lo(trig_enable[i],
                                         wait_for_sys,
                                         rate_only, trig_sign[i],
                                         sma_trig_on_fe[i],
                                         use_coinc,      //use channel coincidence
                                         use_trig_valid, // use trig valid flag as a reset on AC/DC
                                         coinc_window,     //system coincidence window < 15
                                         boardAddress,
                                         device);
                usleep(100);
                Sumo.set_self_trigger_hi(coinc_pulsew,   //coinc pulse width < 7
                                         coinc_num_asic,   //num. asics < 5
                                         coinc_num_ch,   //num. channels < 30
                                         boardAddress,
                                         device);

            }
            if (WRITEACDC) {
                for (int j = 0; j < numChipsOnBoard; j++) {
                    unsigned int chipAddress = pow(2, j % numChipsOnBoard);

                    Sumo.set_pedestal_value(pedestal[i][j], boardAddress, device, chipAddress);
                    usleep(100);
                    Sumo.set_trig_threshold(threshold[i][j], boardAddress, device, chipAddress);
                    usleep(100);
                }
            }
        }
        cout << "__________________________" << std::dec << endl;
        //Sumo.dump_data();
        if (WRITETRIG) {
            if (hrdw_trig) {
                unsigned int ext_trig_mode = 0x0 | 1 << 3 | 1 << 4 | hrdw_trigsrc << 13;
                if (jj == 1)cout << "setting trig mode to master device " << ext_trig_mode << endl;
                Sumo.set_usb_read_mode(ext_trig_mode);
            }

            if (hrdw_trig_sl) {
                unsigned int ext_trig_mode = 0x0 | 1 << 3 | 1 << 4 | hrdw_trig_slsrc << 13;
                if (jj == 1)cout << "setting trig mode to slave device " << ext_trig_mode << endl;
                Sumo.set_usb_read_mode_slaveDevice(ext_trig_mode);
            }
        }
    }


    return 0;
}


// parse parameter file
int parse_acdc_setup_file(const char *file, bool verbose) {
    bool tt = verbose;

    ifstream in;
    in.open(file, ios::in);
    string line, data;

    unsigned int tmp1, tmp2, tmp3;
    bool bool_tmp1;

    while (getline(in, line)) {
        stringstream linestream(line);
        getline(linestream, data, '\t');
        if (data.find("#") == 0)
            continue;
        else if (data.find("pedestal") == 0) {
            linestream >> tmp1 >> tmp3 >> tmp2;
            pedestal[tmp1][tmp3] = tmp2;
            if (tt)
                cout << data << " on board:chip " << tmp1 << ":" << tmp3 << " set to 0x" << hex << tmp2 << endl;
        } else if (data.find("thresh") == 0) {
            linestream >> tmp1 >> tmp3 >> tmp2;
            threshold[tmp1][tmp3] = tmp2;
            if (tt)
                cout << data << " on board:chip " << tmp1 << ":" << tmp3 << " set to 0x" << hex << tmp2 << endl;
        }

    }
    return 0;
}

int parse_trig_setup_file(const char *file, bool verbose) {
    ifstream in;
    in.open(file, ios::in);
    string line, data;

    unsigned int tmp1, tmp2, tmp3;
    bool bool_tmp1;

    while (getline(in, line)) {
        stringstream linestream(line);
        getline(linestream, data, '\t');

        if (data.find("#") == 0)
            continue;

        if (data.find("trig_mask") == 0) {
            linestream >> tmp1 >> hex >> tmp2;
            trig_mask[tmp1] = tmp2;
            if (verbose)
                cout << data << " on board " << tmp1 << " set to 0x" << hex << tmp2 << dec << endl;
        } else if (data.find("trig_enable") == 0) {
            linestream >> tmp1 >> bool_tmp1;
            trig_enable[tmp1] = bool_tmp1;
            if (verbose)
                cout << data << " on board " << tmp1 << " set to " << bool_tmp1 << endl;
        } else if (data.find("trig_sign") == 0) {
            linestream >> tmp1 >> bool_tmp1;
            trig_sign[tmp1] = bool_tmp1;
            if (verbose)
                cout << data << " on board " << tmp1 << " set to " << bool_tmp1 << endl;
        } else if (data.find("wait_for_sys") == 0) {
            linestream >> bool_tmp1;
            wait_for_sys = bool_tmp1;
            cout << data << " set to " << bool_tmp1 << endl;
        } else if (data.find("rate_only") == 0) {
            linestream >> bool_tmp1;
            rate_only = bool_tmp1;
            if (verbose)
                cout << data << " set to " << bool_tmp1 << endl;
        } else if (data.find("hrdw_trig") == 0) {
            linestream >> bool_tmp1;
            hrdw_trig = bool_tmp1;
            cout << "Setting hrdw_trig";
            if (verbose) cout << data << " set to " << hrdw_trig << endl;
        } else if (data.find("hrdw_sl_trig") == 0) {
            linestream >> bool_tmp1;
            hrdw_trig_sl = bool_tmp1;
            if (verbose) cout << data << " set to " << bool_tmp1 << endl;
        } else if (data.find("hrdw_src_trig") == 0) {
            linestream >> tmp1;
            hrdw_trigsrc = tmp1;
            if (verbose) cout << data << " set to " << tmp1 << endl;
        } else if (data.find("hrdw_slsrc_trig") == 0) {
            linestream >> tmp1;
            hrdw_trig_slsrc = tmp1;
            if (verbose) cout << data << " set to " << tmp1 << endl;
        } else if (data.find("sma_trig_on_fe") == 0) {
            linestream >> tmp1 >> bool_tmp1;
            sma_trig_on_fe[tmp1] = bool_tmp1;
            if (verbose) cout << data << " on board " << tmp1 << " set to " << bool_tmp1 << endl;
        } else if (data.find("coinc_window") == 0) {
            linestream >> tmp1;
            coinc_window = tmp1;
            if (verbose) cout << data << " set to " << tmp1 << endl;
        } else if (data.find("coinc_pulsew") == 0) {
            linestream >> tmp1;
            coinc_pulsew = tmp1;
            if (verbose) cout << data << " set to " << tmp1 << endl;
        } else if (data.find("coinc_num_ch") == 0) {
            linestream >> tmp1;
            coinc_num_ch = tmp1;
            if (verbose) cout << data << " set to " << tmp1 << endl;
        } else if (data.find("coinc_num_asic") == 0) {
            linestream >> tmp1;
            coinc_num_asic = tmp1;
            if (verbose) cout << data << " set to " << tmp1 << endl;
        } else if (data.find("use_coinc") == 0) {
            linestream >> bool_tmp1;
            use_coinc = bool_tmp1;
            if (verbose) cout << data << " set to " << bool_tmp1 << endl;
        } else if (data.find("use_trig_valid") == 0) {
            linestream >> bool_tmp1;
            use_trig_valid = bool_tmp1;
            if (verbose) cout << data << " set to " << bool_tmp1 << endl;
        }
    }
    return 0;
}
