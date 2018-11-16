#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

#include "SuMo.h"
#include "Timer.h"

using namespace std;
/* subtract pedestal values on-line */
static bool PED_SUBTRCT = false;

static int LIMIT_READOUT_RATE = 6000;  //usecs limit between event polling
static int NUM_SEQ_TIMEOUTS = 100;    // number of sequential timeouts before ending run
const float MAX_INT_TIMER = 800.;    // max cpu timer before ending run (secs)
/* note: usb timeout defined in include/stdUSB.h */

bool overwriteExistingFile = false;

int SuMo::log_data(unsigned int NUM_READS, int trig_mode, int acq_rate, const char *log_filename) {
    int check_event;
    int asic_baseline[psecSampleCells];
    int count = 0;
    int psec_cnt = 0;
    int last_k;
    float _now_, t = 0.;
    Timer timer = Timer();
    time_t now;
    short sample;
    int *Meta;


    char logDataFilename[300];
    char logMetaFilename[300];
    char logPedFilename[300];
    char delim = ' ';

    // 'scalar' mode
    ofstream rate_fs;
    if (trig_mode == 2) {
        char logRateFilename[300];
        sprintf(logRateFilename, "%s.rate", log_filename);
        rate_fs.open(logRateFilename, ios::trunc);
    }


    sprintf(logDataFilename, "%s.acdc", log_filename);
    sprintf(logMetaFilename, "%s.meta", log_filename);
    sprintf(logPedFilename, "%s.ped", log_filename);

    // check if file exists, inquire whether to overwrite
    string temp;
    while (fileExists(logDataFilename)) {
        cout << "file already exists, try new filename: (or enter to overwrite / ctrl-C to quit): ";
        getline(cin, temp);
        if (temp.empty()) break;
        sprintf(logDataFilename, "%s.acdc", temp.c_str());
        sprintf(logMetaFilename, "%s.meta", temp.c_str());
        sprintf(logPedFilename, "%s.ped", temp.c_str());
    }

    /* open up file stream */
    ofstream dataofs, pedofs, metaofs;
    dataofs.open(logDataFilename, ios::trunc);


    /* Create header */
    dataofs << "Event" << delim << "Board" << delim << "Ch";
    for (int i = 0; i < psecSampleCells; i++) {
        dataofs << delim << i;
    }
    dataofs << endl;

    pedofs.open(logPedFilename, ios::trunc);
    // Create Header
    pedofs << "Board" << delim << "Ch";
    for (int i = 0; i < psecSampleCells; i++) {
        pedofs << delim << i;
    }
    pedofs << endl;

    /* Print out pedestal data */
    for (int board = 0; board < numFrontBoards; board++) {
        // Skip inactive boards
        if (!DC_ACTIVE[board]) continue;
        for (int channel = 0; channel < AC_CHANNELS; channel++) {
            pedofs << board << delim << channel + 1;
            for (int i = 0; i < psecSampleCells; i++) {
                pedofs << delim << PED_DATA[board][channel][i];
            }
            pedofs << endl;
        }
    }

    pedofs.close();

    metaofs.open(logMetaFilename, ios::trunc);

    // Create header
    metaofs << "Event" << delim << "Board" << delim;
    metaofs << "count" << delim << "aa" << delim << "time" << delim << "datetime" << delim
            << "events" << delim << "bin_count_rise" << delim << "self_trig_settings_2" << delim
            << "sys_coincidence_width" << delim << "coincidence_num_chips" << delim
            << "coincidence_num_chans" << delim << "self_trig_settings" << delim
            << "trig_en" << delim << "trig_wait_for_sys" << delim << "trig_rate_only" << delim
            << "trig_sign" << delim << "use_sma_trig_input" << delim
            << "use_coincidence_settings" << delim << "use_trig_valid_as_reset" << delim
            << "coinc_window" << delim << "reg_self_trig" << delim
            << "counts_of_sys_no_local" << delim << "sys_trig_count" << delim
            << "resets_from_firmw" << delim << "firmware_version" << delim
            << "self_trig_mask" << delim << "dig_timestamp_lo" << delim
            << "dig_timestamp_mid" << delim << "dig_timestamp_hi" << delim
            << "dig_event_count" << delim << "event_count" << delim
            << "timestamp_hi" << delim << "timestamp_mid" << delim
            << "timestamp_lo" << delim << "CC_BIN_COUNT" << delim
            << "CC_EVENT_COUNT" << delim << "CC_TIMESTAMP_LO" << delim
            << "CC_TIMESTAMP_MID" << delim << "CC_TIMESTAMP_HI" << delim;
    for (int n = 0; n < 5; n++) metaofs << "ro_cnt_chip_" << n << delim;
    for (int n = 0; n < 5; n++) metaofs << "ro_target_cnt_chip_" << n << delim;
    for (int n = 0; n < 5; n++) metaofs << "vbias_chip_" << n << delim;
    for (int n = 0; n < 5; n++) metaofs << "trigger_threshold_chip_" << n << delim;
    for (int n = 0; n < 5; n++) metaofs << "ro_dac_value_chip_" << n << delim;
    for (int n = 1; n <= AC_CHANNELS; n++) metaofs << "self_trig_scalar_ch_" << n << delim;
    metaofs << "time_from_valid_to_trig" << delim << "firmware_reset_time" << delim
            << "last_coincidence_num_chans" << endl;

    int meta_skip[] = {5, 6, 65, 66, 67, 68, 69, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109};





    // read all front end cards
    bool all[numFrontBoards];
    for (int i = 0; i < numFrontBoards; i++) all[i] = true;

    load_ped();

    int number_of_frontend_cards = 0;

    for (int board = 0; board < numFrontBoards; board++) {
        if (DC_ACTIVE[board]) number_of_frontend_cards++;

    }

    cout << "--------------------------------------------------------------" << endl;
    cout << "number of front-end boards detected = " << number_of_frontend_cards
         << " of " << numFrontBoards << " address slots in system" << endl;
    cout << "Trying for " << NUM_READS << " events logged to disk, in a timeout window of "
         << MAX_INT_TIMER << " seconds" << endl;
    cout << "--------------------------------------------------------------" << endl << endl;

    usleep(100000);


    bool reset_event = true;

    // set read mode to NULL
    set_usb_read_mode(0);
    if (mode == USB2x) set_usb_read_mode_slaveDevice(0);
    system_card_trig_valid(false);
    if (mode == USB2x) system_slave_card_trig_valid(false);

    //check system trigger number
    read_CC(false, false, 100);
    int board_trigger = CC_EVENT_COUNT_FROMCC0;
    int last_board_trigger = board_trigger;

    /* cpu time zero */
    timer.start();

    for (int event = 0; event < NUM_READS; event++) {
        set_usb_read_mode(0);
        if (mode == USB2x) set_usb_read_mode_slaveDevice(0);

        time(&now);
        t = timer.stop();
        // interrupt if past specified logging time
        if (t > MAX_INT_TIMER) {
            cout << endl << "readout timed out at " << t << "on event " << event << endl;
            break;
        }

        // trig_mode = 1 is external source or PSEC4 self trigger
        // trig_mode = 0 is over software (USB), i.e. calibration logging
        if (trig_mode == 0) {
            manage_cc_fifo(1);
            if (mode == USB2x) manage_cc_fifo_slaveDevice(1);

            // 'rate-only' mode, only pull data every second
            if (trig_mode == 2) usleep(3e6);

            prep_sync();
            if (mode == USB2x) software_trigger_slaveDevice((unsigned int) 15);
            software_trigger((unsigned int) 15);
            make_sync();

            //acq rate limit
            usleep(LIMIT_READOUT_RATE); // somewhat arbitrary hard-coded rate limitation
            //system_card_trig_valid(false);
            //if(mode == USB2x) system_slave_card_trig_valid(false);
        } else {
            manage_cc_fifo(1);
            if (mode == USB2x) manage_cc_fifo_slaveDevice(1);
            usleep(100);
            for (int iii = 0; iii < 2; iii++) {
                system_card_trig_valid(false);
                if (mode == USB2x) system_slave_card_trig_valid(false);
            }

            //send in trig 'valid' signal
            sys_wait(100);
            prep_sync();
            if (mode == USB2x) system_slave_card_trig_valid(true);
            system_card_trig_valid(true);
            make_sync();
            //}
            //acq rate limit
            usleep(acq_rate + LIMIT_READOUT_RATE);
        }
        int num_pulls = 0;
        int evts = 0;
        int digs = 0;
        while (board_trigger == last_board_trigger && t < MAX_INT_TIMER) {

            read_CC(false, false, 100);
            board_trigger = CC_EVENT_COUNT_FROMCC0;
            cout << "waiting for trigger...     on system event: "
                 << board_trigger << " & readout attempt " << event
                 << " @time " << t << "                        \r";
            cout.flush();
            usleep(1000);
            t = timer.stop();
            num_pulls++;
            if (num_pulls > 100) break;
            //if(num_pulls > 10)  break;   //use this is board trigger does not iterate
        }

        last_board_trigger = board_trigger;
        if (mode == USB2x) system_slave_card_trig_valid(false);
        system_card_trig_valid(false);
        //set_usb_read_mode_slaveDevice(0), set_usb_read_mode(0);
        evts = read_CC(false, false, 0);
        for (int chkdig = 0; chkdig < numFrontBoards; chkdig++)
            digs += DIGITIZING_START_FLAG[chkdig];

        if (evts == 0 || evts != digs) {
            cout << "    --NULL--       " << endl;
            reset_event = true;
            event = event - 1;             //repeat event
            continue;
        }

            // show event number at terminal
        else {
            if ((event + 1) % 1 == 0 || event == 0) {
                print_to_terminal(event, NUM_READS, CC_EVENT_COUNT_FROMCC0, board_trigger, t);
            }
        }
        /**************************************/
        //Do bulk read on all front-end cards
        int numBoards = read_AC(1, all, false);
        /**************************************/
        sys_wait(10000);

        for (int jj = 0; jj < 2; jj++) {
            //prep_sync();
            //turn off 'trig valid flag' until checking if data in buffer
            if (mode == USB2x) system_slave_card_trig_valid(false);
            system_card_trig_valid(false);
            if (mode == USB2x) set_usb_read_mode_slaveDevice(0);
            set_usb_read_mode(0);
        }
        reset_event = true; //have event, go ahead and reset for next event

        // form data for filesave
        for (int board = 0; board < numFrontBoards; board++) {
            if (BOARDS_READOUT[board] && numBoards > 0) {
                psec_cnt = 0;
                // assign meta data
                Meta = get_AC_info(false, board, false, event, t, t, evts);
                // wraparound_correction
                int baseline[psecSampleCells];
                unwrap_baseline(baseline, board);
                for (int ch = 0; ch < AC_CHANNELS; ch++) {
                    dataofs << event << delim << board << delim << ch + 1;

                    if (ch > 0 && ch % 6 == 0) psec_cnt++;

                    for (int cell = 0; cell < psecSampleCells; cell++) {
                        asic_baseline[cell] = baseline[cell];
                        sample = adcDat[board]->AC_RAW_DATA[psec_cnt][ch % 6 * 256 + cell];
                        sample -= PED_DATA[board][ch][cell];
                        adcDat[board]->Data[ch][baseline[cell]] = (unsigned int) sample;
                    }
                    for (int wrap: adcDat[board]->Data[ch]) {
                        dataofs << delim << dec << wrap; // std::dec
                    }
                    dataofs << endl;
                }

                metaofs << event << delim << board;
                for (int cell = 0; cell < psecSampleCells; cell++) {
                    adcDat[board]->Meta[cell] = Meta[cell];
                    // Only log the meaningful meta values
                    if (cell <= 112 && find(begin(meta_skip), end(meta_skip), cell) == end(meta_skip)) {
                        metaofs << delim << Meta[cell];
                    }
                }
                metaofs << endl;
            }
            // if timeout on only some, but not all boards
            else if (numBoards > 0 && BOARDS_TIMEOUT[board] && DC_ACTIVE[board]) {
                for (int i = 0; i < AC_CHANNELS; i++) {
                    if (i > 0 && i % 6 == 0) psec_cnt++;

                    for (int j = 0; j < psecSampleCells; j++) {
                        sample = 0xFF;
                        adcDat[board]->Data[i][j] = sample;
                    }
                }
            }
        }



        last_k = event;

        // LOGGING
    }

    cout << endl;
    cout << "Done on readout:  " << last_k + 1 << " :: @time " << t << " sec" << endl;

    cleanup();


    dump_data();

    if (trig_mode == 2) rate_fs.close();

    cout << "Data saved in file: " << logDataFilename << endl << "*****" << endl;

    dataofs.close();
    pedofs.close();
    metaofs.close();


    return 0;
}


void SuMo::print_to_terminal(int k, int NUM_READS, int cc_event, int board_trig, double t) {
    cout << "\r" << "Readout:  " << k + 1 << " of " << NUM_READS;
    cout << " :: system|sw evt-" << cc_event << "|" << board_trig;
    cout << " :: evtflags-";
    for (int evt_flag = 0; evt_flag < numFrontBoards; evt_flag++) cout << EVENT_FLAG[evt_flag];
    cout << " :: digflags-";
    for (int dig_flag = 0; dig_flag < numFrontBoards; dig_flag++) cout << DIGITIZING_START_FLAG[dig_flag];
    cout << " :: @time " << t << " sec " << flush;
}
