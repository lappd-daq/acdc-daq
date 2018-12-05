#include "SuMoInterface.h"
#include "config.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
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

/*
 * Does pedestal subtraction
 */
void SuMoInterface::calibrate() {
    int num_checks = 10;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards for calibration");
    }
    sumo.set_usb_read_mode(16);
    sumo.dump_data();
    sumo.generate_ped(true);
}

/*
 * Prints the ACDC status to cout
 */
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


bool SuMoInterface::hasTriggered(bool force) {
    int num_checks = 5;
    if (sumo.check_active_boards(num_checks)) {
        throw runtime_error("Could not initialize boards to ask for trigger");
    }
    sumo.set_usb_read_mode(0);
    sumo.read_CC(false, false, 100);
    int board_trigger = sumo.CC_EVENT_COUNT_FROMCC0;
    int last_trigger = board_trigger;
    sumo.set_usb_read_mode(0);
    sumo.manage_cc_fifo(true);
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
    int num_steps = 0;
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
    }
    sumo.load_ped();

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
            meta_tmp = sumo.get_AC_info(false, board, false, 0, 0, 0, events);
            data.meta.emplace(board, map_metadata(meta_tmp));
            map<int, vector<int>> ch_data;
            for (int ch = 0; ch < AC_CHANNELS; ch++) {
                if (ch > 0 && ch % 6 == 0) psec_cnt++;
                vector<int> trace;
                for (int cell = 0; cell < psecSampleCells; cell++) {
                    sample = sumo.adcDat[board]->AC_RAW_DATA[psec_cnt][ch % 6 * 256 + cell];
                    sample -= sumo.PED_DATA[board][ch][cell];
                    trace.push_back(sample);
                }
                ch_data.emplace(ch+1, trace);
            }
            data.data.emplace(board, ch_data);
        }
    }
    return data;
}

/*
 * Mother of all meta-maps. I want to incorporate this into the base meta parsing in the future (M Lucas)
 */
map<string, unsigned int> map_metadata(int meta_array[]) {
    map<string, unsigned int> meta;
    meta.emplace("count", meta_array[0]);
    meta.emplace("aa", meta_array[1]);
    meta.emplace("time", meta_array[2]);
    meta.emplace("datetime", meta_array[3]);
    meta.emplace("events", meta_array[4]);
    meta.emplace("bin_count_rise", meta_array[7]);
    meta.emplace("self_trig_settings_2", meta_array[8]);
    meta.emplace("sys_coinc_width", meta_array[9]);
    meta.emplace("coinc_num_chips", meta_array[10]);
    meta.emplace("coinc_num_chans", meta_array[11]);
    meta.emplace("self_trig_settings", meta_array[12]);
    meta.emplace("trig_en", meta_array[13]);
    meta.emplace("trig_wait_for_sys", meta_array[14]);
    meta.emplace("trig_rate_only", meta_array[15]);
    meta.emplace("trig_sign", meta_array[16]);
    meta.emplace("use_sma_trig_en", meta_array[17]);
    meta.emplace("use_coinc_settings", meta_array[18]);
    meta.emplace("use_trig_valid_as_reset", meta_array[19]);
    meta.emplace("coinc_window", meta_array[20]);
    meta.emplace("reg_self_trig", meta_array[21]);
    meta.emplace("counts_of_sys_no_local", meta_array[22]);
    meta.emplace("sys_trig_count", meta_array[23]);
    meta.emplace("resets_from_firmw", meta_array[24]);
    meta.emplace("firmware_version", meta_array[25]);
    meta.emplace("self_trig_mask", meta_array[26]);
    meta.emplace("dig_timiestamp_lo", meta_array[27]);
    meta.emplace("dig_timestamp_mid", meta_array[28]);
    meta.emplace("dig_timestamp_hi", meta_array[29]);
    meta.emplace("dig_event_count", meta_array[30]);
    meta.emplace("event_count", meta_array[31]);
    meta.emplace("timestamp_hi", meta_array[32]);
    meta.emplace("timestamp_mid", meta_array[33]);
    meta.emplace("timestamp_lo", meta_array[34]);
    meta.emplace("CC_bin_count", meta_array[35]);
    meta.emplace("CC_event_count", meta_array[36]);
    meta.emplace("CC_timestamp_lo", meta_array[37]);
    meta.emplace("CC_timestamp_mid", meta_array[38]);
    meta.emplace("CC_timestamp_hi", meta_array[39]);
    stringstream tmp;
    for (int i = 0; i < numChipsOnBoard; i++) {
        tmp << "RO_count_" << i;
        meta.emplace(tmp.str(), meta_array[40+i]);
        tmp.str("");
        tmp << "RO_target_count_" << i;
        meta.emplace(tmp.str(), meta_array[45+i]);
        tmp.str("");
        tmp << "vbias_" << i;
        meta.emplace(tmp.str(), meta_array[50+i]);
        tmp.str("");
        tmp << "trigger_threshold_" << i;
        meta.emplace(tmp.str(), meta_array[55+i]);
        tmp.str("");
        tmp << "RO_DAC_value_" << i;
        meta.emplace(tmp.str(), meta_array[60+i]);
        tmp.str();
    }
    for (int i = 0; i < AC_CHANNELS; i++) {
        tmp << "self_trig_scalar_" << i;
        meta.emplace(tmp.str(), meta_array[70+i]);
    }
    meta.emplace("time_from_valid_to_trig", meta_array[110]);
    meta.emplace("firmware_reset_time", meta_array[111]);
    meta.emplace("last_coinc_num_chans", meta_array[112]);
    return meta;
}

void SuMoInterface::reset() {
    // TODO
}

void SuMoInterface::data_to_csv(vector<SumoData> data, string filename) {
    string tmp;
    while (SuMo::fileExists(filename)) {
        cout << "'" << filename << "' already exists, would you like to overwrite? [yN]: ";
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
    int board, ch;
    for (int event = 0; event < data.size(); event++) {
        SumoData datum = data[event];
        for (const auto& boards: datum.data) {
            board = boards.first;
                for (const auto& channels: boards.second) {
                    ch = channels.first;
                    out << event << delim << board << delim << ch;
                    for (int cell : channels.second) {
                        out << delim << cell;
                    }
                    out << endl;
                }

        }
    }
}

void SuMoInterface::meta_to_csv(vector<SumoData> data, string filename) {
    string tmp;
    while (SuMo::fileExists(filename)) {
        cout << "'" << filename << "' already exists, would you like to overwrite? [yN]: ";
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

    char delim = ',';
    int board;
    for (int event = 0; event < data.size(); event++) {
        SumoData datum = data[event];
        for (const auto& boards : datum.meta) {
            if (event == 0) {
                out << "Event" << delim << "Board";
                for (const auto& meta : boards.second) {
                    out << delim << meta.first;
                }
                out << endl;
            }
            board = boards.first;
            out << event << delim << board;
            for (const auto& meta : boards.second) {
                out << delim << meta.second;
            }
            out << endl;
        }
    }

}






