#include "SuMoInterface.h"
#include "config.h"
#include <stdexcept>
#include <sys/stat.h>
#include <iostream>
#include "Timer.h"
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
    if (!sumo.fileExists(filename)) {
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

    sumo.sys_wait(1000);
    sumo.dump_data();
}

void SuMoInterface::prepare() {
    sumo.set_usb_read_mode(0);
    sumo.manage_cc_fifo(true);
    sumo.system_card_trig_valid(false);
    sumo.sys_wait(100);
    sumo.prep_sync();
    sumo.system_card_trig_valid(true);
    sumo.make_sync();
    last_trigger = sumo.CC_EVENT_COUNT_FROMCC0;
    is_prepared = true;
}

bool SuMoInterface::hasTriggered() {
    if (!is_prepared) {
        throw runtime_error("must call SuMoInterface::prepare before any triggering logic");
    }
    sumo.read_CC(false, false, 100);
    int board_trigger = sumo.CC_EVENT_COUNT_FROMCC0;
    if (board_trigger != last_trigger) {
        last_trigger = board_trigger;
        return true;
    } else {
        return false;
    }
}

packet_t SuMoInterface::getData() {
    int events = sumo.read_CC(false, false, 0);


}







