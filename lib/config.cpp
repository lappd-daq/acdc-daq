#include "config.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include "yaml-cpp/yaml.h"

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
bool hrdw_sl_trig;
unsigned int hrdw_trig_src;
unsigned int hrdw_sl_trig_src;

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
    hrdw_sl_trig = false;
    hrdw_trig_src = 0;
    hrdw_sl_trig_src = 0;
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
                unsigned int ext_trig_mode = 0x0 | 1 << 3 | 1 << 4 | hrdw_trig_src << 13;
                if (jj == 1)cout << "setting trig mode to master device " << ext_trig_mode << endl;
                Sumo.set_usb_read_mode(ext_trig_mode);
            }

            if (hrdw_sl_trig) {
                unsigned int ext_trig_mode = 0x0 | 1 << 3 | 1 << 4 | hrdw_sl_trig_src << 13;
                if (jj == 1)cout << "setting trig mode to slave device " << ext_trig_mode << endl;
                Sumo.set_usb_read_mode_slaveDevice(ext_trig_mode);
            }
        }
    }


    return 0;
}


int parse_setup_file(const char *file, bool verbose) {
    // ACDC Settings
    YAML::Node config = YAML::LoadFile(file);
    int retval = 0;
    if (config["acdc_settings"]) {
        retval = parse_acdc_setup_yaml(config["acdc_settings"], verbose);

    }
    if (retval != 0) {
        return retval;
    }
    if (config["trigger_settings"]) {
        retval = parse_trig_setup_yaml(config["trigger_settings"], verbose);

    }
    return retval;
}

// parse parameter file
int parse_acdc_setup_yaml(YAML::Node config, bool verbose) {
    int board, chip;
    if (verbose) {
        cout << "__________________________" << endl;
    }
    if (config["pedestal"]) {
        YAML::Node pedestals = config["pedestal"];
        for (YAML::const_iterator ped = pedestals.begin(); ped != pedestals.end(); ++ped) {
            board = ped->first.as<int>();
            for (YAML::const_iterator chips = ped->second.begin(); chips != ped->second.end(); ++chips) {
                if (chips->first.as<string>() == "all") {
                    for (int c = 0; c < numChipsOnBoard; c++) {
                        pedestal[board][c] = chips->second.as<unsigned int>();
                        if (verbose) {
                            cout << "Pedestal  on " << board << ":" << c << " set to " << pedestal[board][c] << endl;
                        }
                    }
                    break; // short-circuit
                } else {
                    chip = chips->first.as<int>();
                    pedestal[board][chip] = chips->second.as<unsigned int>();
                    if (verbose) {
                        cout << "Pedestal  on " << board << ":" << chip << " set to " << pedestal[board][chip] << endl;
                    }
                }
            }

        }
    }

    if (verbose) {
        cout << "__________________________" << endl;
    }

    if (config["threshold"]) {
        YAML::Node thresholds = config["threshold"];
        for (YAML::const_iterator thresh = thresholds.begin(); thresh != thresholds.end(); ++thresh) {
            board = thresh->first.as<int>();
            for (YAML::const_iterator chips = thresh->second.begin(); chips != thresh->second.end(); ++chips) {
                if (chips->first.as<string>() == "all") {
                    for (int c = 0; c < numChipsOnBoard; c++) {
                        threshold[board][c] = chips->second.as<unsigned int>();
                        if (verbose) {
                            cout << "Threshold on " << board << ":" << c << " set to " << threshold[board][c] << endl;
                        }
                    }
                    break; // Short-circuit
                } else {
                    chip = chips->first.as<int>();
                    threshold[board][chip] = chips->second.as<unsigned int>();
                    if (verbose) {
                        cout << "Threshold on " << board << ":" << chip << " set to " << pedestal[board][chip] << endl;
                    }
                }
            }

        }
    }
    if (verbose) {
        cout << "__________________________" << endl;
    }
    return 0;
}

int parse_trig_setup_yaml(YAML::Node config, bool verbose) {
    if (verbose) {
        cout << "__________________________" << endl;
    }
    if (config["trig_mask"]) {
        YAML::Node masks = config["trig_mask"];
        for (YAML::const_iterator board = masks.begin(); board != masks.end(); ++board) {
            trig_mask[board->first.as<int>()] = board->second.as<unsigned int>();
            if (verbose) {
                cout << "trig_mask on board " << board->first.as<int>() << " set to 0x" << hex
                     << board->second.as<unsigned int>() << endl;
            }
        }
    }
    if (config["trig_enable"]) {
        YAML::Node flags = config["trig_enable"];
        for (YAML::const_iterator board = flags.begin(); board != flags.end(); ++board) {
            trig_enable[board->first.as<int>()] = board->second.as<bool>();
            if (verbose) {
                cout << "trig_enable on board " << board->first.as<int>() << " set to " << board->second.as<bool>()
                     << endl;
            }
        }
    }
    if (config["trig_sign"]) {
        YAML::Node signs = config["trig_sign"];
        for (YAML::const_iterator board = signs.begin(); board != signs.end(); ++board) {
            string sign = board->second.as<string>();
            trig_sign[board->first.as<int>()] = sign == "rising";
            if (verbose) {
                cout << "trig_sign on board " << board->first.as<int>() << " set to " << sign << endl;
            }
        }
    }
    if (config["wait_for_sys"]) {
        wait_for_sys = config["wait_for_sys"].as<bool>();
        if (verbose) {
            cout << "wait_for_sys set to " << wait_for_sys << endl;
        }
    }
    if (config["rate_only"]) {
        rate_only = config["rate_only"].as<bool>();
        if (verbose) {
            cout << "rate_only set to " << rate_only << endl;
        }
    }
    if (config["hrdw_trig"]) {
        hrdw_trig = config["hrdw_trig"].as<bool>();
        if (verbose) {
            cout << "hrdw_trig set to " << hrdw_trig << endl;
        }
    }
    if (config["hrdw_trig_src"]) {
        hrdw_trig_src = config["hrdw_trig_src"].as<unsigned int>();
        if (verbose) {
            cout << "hrdw_trig_src set to " << hrdw_trig_src << endl;
        }
    }
    if (config["hrdw_sl_trig"]) {
        hrdw_sl_trig = config["hrdw_sl_trig"].as<bool>();
        if (verbose) {
            cout << "hrdw_sl_trig set to " << hrdw_sl_trig << endl;
        }
    }
    if (config["hrdw_sl_trig_src"]) {
        hrdw_sl_trig_src = config["hrdw_sl_trig_src"].as<unsigned int>();
        if (verbose) {
            cout << "hrdw_sl_trig_src set to " << hrdw_sl_trig_src << endl;
        }
    }
    if (config["sma_trig_on_fe"]) {
        YAML::Node flags = config["sma_trig_on_fe"];
        for (YAML::const_iterator board = flags.begin(); board != flags.end(); ++board) {
            sma_trig_on_fe[board->first.as<int>()] = board->second.as<bool>();
            if (verbose) {
                cout << "sma_trig_on_fe on board " << board->first.as<int>() << " set to " << board->second.as<bool>()
                     << endl;
            }
        }
    }
    if (config["use_coinc"]) {
        use_coinc = config["use_coinc"].as<bool>();
        if (verbose) {
            cout << "use_coinc set to " << use_coinc << endl;
        }
    }
    if (config["coinc_window"]) {
        coinc_window = config["coinc_window"].as<unsigned int>();
        if (verbose) {
            cout << "coinc_window set to " << coinc_window << endl;
        }
    }
    if (config["coinc_pulsew"]) {
        coinc_pulsew = config["coinc_pulsew"].as<unsigned int>();
        if (verbose) {
            cout << "coinc_pulsew set to " << coinc_pulsew << endl;
        }
    }
    if (config["coinc_num_ch"]) {
        coinc_num_ch = config["coinc_num_ch"].as<unsigned int>();
        if (verbose) {
            cout << "coinc_num_ch set to " << coinc_num_ch << endl;
        }
    }
    if (config["coinc_num_asic"]) {
        coinc_num_asic = config["coinc_num_asic"].as<unsigned int>();
        if (verbose) {
            cout << "coinc_num_asic set to " << coinc_num_asic << endl;
        }
    }
    if (config["use_trig_valid"]) {
        use_trig_valid = config["use_trig_valid"].as<bool>();
        if (verbose) {
            cout << "use_trig_valid set to " << use_trig_valid << endl;
        }
    }
    return 0;
}
