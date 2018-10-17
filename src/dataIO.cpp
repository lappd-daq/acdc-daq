#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <fstream>

#include "SuMo.h"
#include "Timer.h"

using namespace std;
/* subtract pedestal values on-line */
static bool PED_SUBTRCT = false;

static int LIMIT_READOUT_RATE  = 6000;  //usecs limit between event polling
static int NUM_SEQ_TIMEOUTS    = 100;    // number of sequential timeouts before ending run
const  float  MAX_INT_TIMER    = 800.;    // max cpu timer before ending run (secs)
/* note: usb timeout defined in include/stdUSB.h */

bool overwriteExistingFile = false;

vector<packet_t**> SuMo::get_data(unsigned int NUM_READS, int trig_mode, int acq_rate){
    int check_event;
    int asic_baseline[psecSampleCells];
    int count = 0;
    int psec_cnt = 0;
    int last_k;
    float  _now_, t = 0.;
    char logDataFilename[300];
    Timer timer = Timer();
    time_t now;
    unsigned short sample;
    int* Meta;

    vector<packet_t**> event_data;

    // read all front end cards
    bool all[numFrontBoards];
    for(int i = 0; i < numFrontBoards; i++) all[i] = true;

    load_ped();

    int number_of_frontend_cards = 0;

    for( int board = 0; board < numFrontBoards; board++){
        if(DC_ACTIVE[board]) number_of_frontend_cards++;

    }

    cout << "--------------------------------------------------------------" << endl;
    cout << "number of front-end boards detected = " << number_of_frontend_cards
         << " of " << numFrontBoards << " address slots in system" << endl;
    cout << "Trying for " << NUM_READS << " events logged to disk, in a timeout window of "
         << MAX_INT_TIMER << " seconds" << endl;
    cout << "--------------------------------------------------------------" << endl << endl;

    usleep(100000);

    /* cpu time zero */
    timer.start();

    bool reset_event = true;

    // set read mode to NULL
    set_usb_read_mode(0);
    if(mode==USB2x) set_usb_read_mode_slaveDevice(0);
    system_card_trig_valid(false);
    if(mode==USB2x) system_slave_card_trig_valid(false);

    //check system trigger number
    read_CC(false, false, 100);
    int board_trigger= CC_EVENT_COUNT_FROMCC0;
    int last_board_trigger = board_trigger;

    for(int k=0; k<NUM_READS; k++){
        set_usb_read_mode(0);
        if(mode==USB2x) set_usb_read_mode_slaveDevice(0);

        time(&now);
        t = timer.stop();
        // interrupt if past specified logging time
        if(t > MAX_INT_TIMER) {
            cout << endl << "readout timed out at " << t << "on event " << k << endl;
            break;
        }

        // trig_mode = 1 is external source or PSEC4 self trigger
        // trig_mode = 0 is over software (USB), i.e. calibration logging
        if(trig_mode == 0){
            manage_cc_fifo(1);
            if(mode==USB2x) manage_cc_fifo_slaveDevice(1);

            // 'rate-only' mode, only pull data every second
            if(trig_mode == 2) usleep(3e6);

            prep_sync();
            if(mode == USB2x) software_trigger_slaveDevice((unsigned int)15);
            software_trigger((unsigned int) 15);
            make_sync();

            //acq rate limit
            usleep(LIMIT_READOUT_RATE); // somewhat arbitrary hard-coded rate limitation
            //system_card_trig_valid(false);
            //if(mode == USB2x) system_slave_card_trig_valid(false);
        }
        else{
            manage_cc_fifo(1);
            if(mode==USB2x) manage_cc_fifo_slaveDevice(1);
            usleep(100);
            for(int iii=0; iii<2; iii++){
                system_card_trig_valid(false);
                if(mode==USB2x) system_slave_card_trig_valid(false);
            }

            //send in trig 'valid' signal
            sys_wait(100);
            prep_sync();
            if(mode==USB2x) system_slave_card_trig_valid(true);
            system_card_trig_valid(true);
            make_sync();
            //}
            //acq rate limit
            usleep(acq_rate+LIMIT_READOUT_RATE);
        }
        int num_pulls = 0;
        int evts = 0;
        int digs = 0;
        while(board_trigger==last_board_trigger && t < MAX_INT_TIMER){

            read_CC(false, false, 100);
            board_trigger = CC_EVENT_COUNT_FROMCC0;
            cout << "waiting for trigger...     on system event: "
                 << board_trigger << " & readout attempt " << k
                 << " @time " << t << "                        \r";
            cout.flush();
            usleep(1000);
            t = timer.stop();
            num_pulls++;
            if(num_pulls > 100) break;
            //if(num_pulls > 10)  break;   //use this is board trigger does not iterate
        }

        last_board_trigger = board_trigger;
        if(mode == USB2x) system_slave_card_trig_valid(false);
        system_card_trig_valid(false);
        //set_usb_read_mode_slaveDevice(0), set_usb_read_mode(0);
        evts = read_CC(false, false, 0);
        for(int chkdig=0; chkdig<numFrontBoards; chkdig++)
            digs+=DIGITIZING_START_FLAG[chkdig];

        //if(evts == 0){
        //reset_event = true;
        //k = k-1;
        //continue;
        //}
        //condition for dumping event and trying again.
        //else if( DIGITIZING_START_FLAG[2] == 0 || evts < 6 || evts != digs){
        if( evts == 0 || evts != digs){
            print_to_terminal(k, NUM_READS,CC_EVENT_COUNT_FROMCC0, board_trigger, t);
            cout << "    --NULL--       " << endl;
            //cout.flush();
            reset_event = true;
            k = k-1;             //repeat event
            continue;
        }

            // show event number at terminal
        else{
            if((k+1) % 1 == 0 || k==0){
                print_to_terminal(k, NUM_READS, CC_EVENT_COUNT_FROMCC0, board_trigger, t);
                cout << "          \r";
                cout.flush();
            }
        }
        /**************************************/
        //Do bulk read on all front-end cards
        int numBoards = read_AC(1, all, false);
        /**************************************/
        sys_wait(10000);

        for(int jj=0; jj<2; jj++){
            //prep_sync();
            //turn off 'trig valid flag' until checking if data in buffer
            if(mode == USB2x) system_slave_card_trig_valid(false);
            system_card_trig_valid(false);
            if(mode == USB2x) set_usb_read_mode_slaveDevice(0);
            set_usb_read_mode(0);
        }
        reset_event = true; //have event, go ahead and reset for next event

        // form data for filesave
        for(int targetAC = 0; targetAC < numFrontBoards; targetAC++){
            if(BOARDS_READOUT[targetAC] && numBoards > 0){
                psec_cnt = 0;
                // assign meta data
                Meta=get_AC_info(false, targetAC, false,k, t, t, evts);

                check_event = 0;

                for(int i = 0; i < AC_CHANNELS; i++){
                    if(i>0 && i % 6 == 0) psec_cnt ++;

                    for(int j = 0; j < psecSampleCells; j++){
                        sample = adcDat[targetAC]->AC_RAW_DATA[psec_cnt][i%6*256+j];
                        if(PED_SUBTRCT) sample -= PED_DATA[targetAC][i][j];

                        adcDat[targetAC]->Data[i][j] = (unsigned int) sample;
                    }
                }
                /* wraparound_correction, if desired: */
                int baseline[psecSampleCells];
                unwrap_baseline(baseline, 2);
                for (int j = 0; j < psecSampleCells; j++){
                    asic_baseline[j] = baseline[j];
                    adcDat[targetAC]->Meta[j] = Meta[j];
                }
            }
                //if timeout on only some, but not all boards
            else if( numBoards > 0 && BOARDS_TIMEOUT[targetAC] && DC_ACTIVE[targetAC]){
                for(int i = 0; i < AC_CHANNELS; i++){
                    if(i>0 && i % 6 == 0) psec_cnt ++;

                    for(int j = 0; j < psecSampleCells; j++){
                        sample = 0xFF;
                        adcDat[targetAC]->Data[i][j] = sample;
                    }
                }
                for(int j = 0; j < psecSampleCells; j++){
                    adcDat[targetAC]->Data[AC_CHANNELS][j] =0;
                }
            }
        }
        event_data.push_back(adcDat);


        last_k = k;
    }

    cout << endl;
    cout << "Done on readout:  " << last_k+1 << " :: @time " << t << " sec" << endl;

    cleanup();


    dump_data();

    return event_data;
}

int SuMo::log_data(const char* log_filename, std::vector<packet_t**> event_data, int trig_mode){
    int asic_baseline[psecSampleCells];
    float  _now_, t = 0.;
    Timer timer = Timer();
    time_t now;
    int last_k;
    char logDataFilename[300];
    char logMetaFilename[300];
    char logPedFilename[300];
    char delim = ' ';

    // 'scalar' mode
    ofstream rate_fs;
    if(trig_mode == 2){
        char logRateFilename[300];
        sprintf(logRateFilename, "%s.rate", log_filename);
        rate_fs.open(logRateFilename, ios::trunc);
    }

    sprintf(logDataFilename, "%s.acdc", log_filename);
    sprintf(logMetaFilename, "%s.meta", log_filename);
    sprintf(logPedFilename, "%s.ped", log_filename);

    // check if file exists, inquire whether to overwrite
    string temp;
    while(fileExists(logDataFilename)){
        cout << "file already exists, try new filename: (or enter to overwrite / ctrl-C to quit): ";
        getline(cin, temp);
        if(temp.empty()) break;
        sprintf(logDataFilename, "%s.acdc", temp.c_str());
        sprintf(logMetaFilename, "%s.meta", temp.c_str());
        sprintf(logPedFilename, "%s.ped", temp.c_str());
    }

    /* open up file stream */
    ofstream ofs;
    ofs.open(logDataFilename, ios::trunc);

    /* wraparound_correction, if desired: */
    int baseline[psecSampleCells];
    unwrap_baseline(baseline, 2);
    for (int j = 0; j < psecSampleCells; j++){
        asic_baseline[j] = baseline[j];
    }

    /* Create header */
    ofs << "Event" << delim << "Board" << delim << "Ch";
    for(int i = 0; i < psecSampleCells; i++){
        ofs << delim << "C" << i;
    }
    ofs << endl;

    /* Record events */
    // For each event
    for(int event = 0; event < event_data.size(); event++){
        packet_t** events = event_data[event];
        // For each board
        for (int board = 0; board < numFrontBoards; board++){
            if (!DC_ACTIVE[board]) continue;
            // For each channel
            for (int ch = 0; ch < AC_CHANNELS; ch++) {
                ofs << event << delim << board << delim << ch + 1;
                // For each sample
                for (int i = 0; i < psecSampleCells; i++){
                    int ped_subtracted = events[board]->Data[ch][i] - PED_DATA[board][ch][i];
                    ofs << delim << dec << ped_subtracted; // std::dec
                }
                ofs << endl;
            }
        }

        /* I don't know what this is supposed to do */
        if(trig_mode == 2){
            for(int board=0; board<numFrontBoards; board++){
                // Get the event vector for the given board
                if (DC_ACTIVE[board]){
                    if(BOARDS_READOUT[board]){
                        rate_fs << event << "\t" << board << "\t" << t << "\t";
                        for(int channel=0; channel < AC_CHANNELS; channel++)  rate_fs <<  events[board]->self_trig_scalar[channel] << "\t";

                        rate_fs << endl;
                    }
                }
            }
        }
        last_k = event;
    }

    /* Print out pedestal data */
    ofs.close();
    ofs.open(logPedFilename, ios::trunc);
    // Create Header
    ofs << "Board" << delim << "Ch";
    for(int i = 0; i < psecSampleCells; i++){
        ofs << delim << "C" << i;
    }
    ofs << endl;

    for (int board=0; board<numFrontBoards; board++){
        // Skip inactive boards
        if (!DC_ACTIVE[board]) continue;
        for (int channel=0; channel < AC_CHANNELS; channel++){
            ofs << board << delim << channel + 1;
            for (int i=0; i < psecSampleCells; i++){
                ofs << delim << PED_DATA[board][channel][i];
            }
            ofs << endl;
        }
    }

    /* Print out meta data */
    ofs.close();
    ofs.open(logMetaFilename, ios::trunc);

    // Create header
    ofs << "Event" << delim << "Board" << delim;
    ofs << "count" << delim << "aa" << delim << "time" << delim << "datetime" << delim
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
    for (int n = 0; n < 5; n++) ofs << "ro_cnt_chip_" << n << delim;
    for (int n = 0; n < 5; n++) ofs << "ro_target_cnt_chip_" << n << delim;
    for (int n = 0; n < 5; n++) ofs << "vbias_chip_" << n << delim;
    for (int n = 0; n < 5; n++) ofs << "trigger_threshold_chip_" << n << delim;
    for (int n = 0; n < 5; n++) ofs << "ro_dac_value_chip_" << n << delim;
    for (int n = 1; n <= AC_CHANNELS; n++) ofs << "self_trig_scalar_ch_" << n << delim;
    ofs << "time_from_valid_to_trig" << delim << "firmware_reset_time" << delim
        << "last_coincidence_num_chans" << endl;

    for(int event = 0; event < event_data.size(); event++){
        packet_t** events = event_data[event];
        // For each board
        for (int board = 0; board < numFrontBoards; board++){
            if (!DC_ACTIVE[board]) continue;
            ofs << event << delim << board;

            for (int i = 0; i < psecSampleCells; i++){
                ofs << delim << events[board]->Meta[i];
            }
            ofs << endl;
        }
    }
    cout << endl;
    cout << "Done on readout:  " << last_k+1 << " :: @time " <<t<< " sec" << endl;

    ofs.close();
    cleanup();


    if(trig_mode == 2) rate_fs.close();

    cout << "Data saved in file: " << logDataFilename << endl << "*****" << endl;


    return 0;
}


void SuMo::print_to_terminal(int k, int NUM_READS, int cc_event, int board_trig, double t){
    cout << "Readout:  " << k+1 << " of " << NUM_READS;
    cout <<" :: system|sw evt-" << cc_event << "|" << board_trig;
    cout <<" :: evtflags-";
    for(int evt_flag =0; evt_flag< numFrontBoards; evt_flag++) cout << EVENT_FLAG[evt_flag];
    cout << " :: digflags-";
    for(int dig_flag =0; dig_flag< numFrontBoards; dig_flag++) cout << DIGITIZING_START_FLAG[dig_flag];
    cout <<" :: @time "<< t << " sec ";
    //cout.flush();
}
