/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// setTrig.cpp
// written: ejo
/////////////////////////////////

#include "config.h"
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

/* specific to file */
const int NUM_ARGS = 2;
const char *filename = "setTrig";
const char *description = "setup trigger, parse trigger parms file (argv[1])";
const char *arg_desc = "[<configfile.yml>] [-v]";


int main(int argc, char *argv[]) {
    if (argc == 2 && std::string(argv[1]) == "-h") {
        cout << endl;
        cout << filename << " :: " << description << endl;
        cout << filename << " :: takes " << NUM_ARGS - 1 << " arguments" << endl;
        cout << "Usage: ./bin/" << filename << " " << arg_desc << endl;
        return 1;
    } else if (argc > NUM_ARGS + 1) {
        cout << "error: wrong number of arguments" << endl;
        cout << "Usage: ./bin/" << filename << " " << arg_desc << endl;
        return -1;
    }
        /* function defined below */
    else {
        bool verbose = false;
        if (argc == NUM_ARGS + 1) {
            if (std::string(argv[NUM_ARGS]) == "-v") {
                verbose = true;
            } else {
                cout << "Unrecognized input " << argv[NUM_ARGS] << endl;
                return -1;
            }
        }
        SuMo Sumo;
        int num_checks = 5;
        char paramsFile[200];

        if (Sumo.check_active_boards(num_checks)) return 1;
        if (argc > 1) {
            strcpy(paramsFile, argv[1]);
            parse_setup_file(paramsFile, verbose);
            write_config_to_hardware(Sumo, true, true);
            return 0;
        } else if (argc == 1) {
            cout << "Setting default (trig off) values" << endl;
            set_default_values();
            write_config_to_hardware(Sumo, true, true);
            return 0;
        } else {
            cout << "error, nothing was done" << endl;
            return -1;
        }

    }
}
