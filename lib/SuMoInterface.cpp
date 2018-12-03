#include "SuMoInterface.h"
#include "config.h"
#include <stdexcept>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
using namespace std;

/*
 * Sets the default configuration scheme
 */
void SuMoInterface::configure() {
    int num_checks = 5;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards for configuration");
    }
    set_default_values();
    write_config_to_hardware(sumo, true, true);
}


/*
 * Sets the configuration outlined in <filename> which should be a yaml based specification
 */
void SuMoInterface::configure(string filename, bool verbose) {
    // Check if file exists
    if (!SuMo::fileExists(filename)) {
        throw invalid_argument("filename does not point to a valid file");
    }

    int num_checks = 5;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards for configuration");
    }

    parse_setup_file(filename.c_str(), verbose);
    write_config_to_hardware(sumo, true, true);
}

void SuMoInterface::calibrate() {
    int num_checks = 10;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards for calibration");
    }
    sumo.set_usb_read_mode(16);
    sumo.dump_data();
    sumo.generate_ped(true);
}

void SuMoInterface::getStatus() {
    int num_checks = 3;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards to get status");
    }
    sumo.read_CC(true, true, 0);
    int mode = sumo.check_readout_mode();
    if (mode == 1 && sumo.check_active_boards_slaveDevice() > 0) {
        cout << "slave board detected" << endl;
        sumo.read_CC(true, true, 1, 0);
    }

    SuMo::sys_wait(1000);
    sumo.dump_data();
}

void SuMoInterface::prepare() {
    int num_checks = 5;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards to get status");
    }
    sumo.set_usb_read_mode(0);
    sumo.read_CC(false, false, 100);
    board_trigger = sumo.CC_EVENT_COUNT_FROMCC0;
    last_trigger = board_trigger;
    is_prepared = true;
    sumo.set_usb_read_mode(0);
    sumo.manage_cc_fifo(true);
}

bool SuMoInterface::hasTriggered(bool force) {
    if (force) {
        sumo.prep_sync();
        sumo.software_trigger(15);
    } else {
        sumo.system_card_trig_valid(false);
        sumo.prep_sync();
        sumo.system_card_trig_valid(true);
    }
    sumo.make_sync();
    SuMo::sys_wait(6000);
    int num_steps;
    while(board_trigger == last_trigger) {
        sumo.read_CC(false, false, 100);
        board_trigger = sumo.CC_EVENT_COUNT_FROMCC0;
        if (num_steps++ > 100) {
            return true;
        }
    }

    return (board_trigger != last_trigger);
}

SumoData SuMoInterface::getData() {
    bool all[numFrontBoards];
    for (int i = 0; i < numFrontBoards; i++){
        all[i] = true;
        board_mask[i] = false;
    }

    sumo.load_ped();

    int number_of_frontend_cards = 0;

    for (int board = 0; board < numFrontBoards; board++) {
        if (sumo.DC_ACTIVE[board]) number_of_frontend_cards++;
    }
    SumoData data;
    int digs = 0;
    sumo.system_card_trig_valid(false);
    sumo.set_usb_read_mode(0);
    int events = sumo.read_CC(false, false, 0);
    for (int checkDigit = 0; checkDigit < numFrontBoards; checkDigit++) {
        digs += sumo.DIGITIZING_START_FLAG[checkDigit];
    }
    if (events == 0 || events != digs) {
        throw runtime_error("Invalid readout. Data will be dumped.");
    }
    int *meta_tmp;
    short sample;
    int psec_cnt = 0;
    int numBoards = sumo.read_AC(1, all, false);
    for (int board = 0; board < numFrontBoards; board++) {
        if(sumo.BOARDS_READOUT[board] && numBoards > 0) {
            board_mask[board] = true;
            meta_tmp = sumo.get_AC_info(false, board, false, 0, 0, 0, events);
            for (int ch = 0; ch < AC_CHANNELS; ch++) {
                if (ch > 0 && ch % 6 == 0) psec_cnt++;
                for (int cell = 0; cell < psecSampleCells; cell++) {
                    data.metaData[board][cell] = meta_tmp[cell];
                    sample = sumo.adcDat[board]->AC_RAW_DATA[psec_cnt][ch % 6 * 256 + cell];
                    sample -= sumo.PED_DATA[board][ch][cell];
                    data.data[board][ch][cell] = sample;
                }
            }

        }
    }
    return data;
}

void SuMoInterface::reset() {
    // TODO
}

void SuMoInterface::to_csv(vector<SumoData> data, string filename) {
    string tmp;
    while (SuMo::fileExists(filename)) {
        cout << "File already exists, would you like to overwrite? [yN]" << endl;
        getline(cin, tmp);
        if (tmp == "y") {
            break;
        } else {
            cout << "Enter new filename: ";
            getline(cin, tmp);
            filename = tmp;
        }
    }

    ofstream out;
    out.open(filename, ios::trunc);

    /* Create header */
    char delim = ',';
    out << "Event" << delim << "Board" << delim << "Ch";
    for (int i = 0; i < psecSampleCells; i++) {
        out << delim << i;
    }
    out << endl;
    for (int event = 0; event < data.size(); event++) {
        SumoData datum = data[event];
        for (int board = 0; board < numFrontBoards; board++) {
            if (board_mask[board]) {
                for (int ch = 0; ch < AC_CHANNELS; ch++) {
                    out << event << delim << board << delim << ch + 1;
                    for (int cell = 0; cell < psecSampleCells; cell++) {
                        out << delim << datum.data[board][ch][cell];
                    }
                    out << endl;
                }
            }
        }
    }
}







