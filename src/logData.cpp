#include <iostream>
#include <cstring>

#include "SuMo.h"
#include "Timer.h"

/* specific to file */
const int NUM_ARGS = 4;
const char* filename = "logData";
const char* description = "log data from DAQ";
const char* arg_desc = " <output filename prefix> <num events> <trigger mode 0/1/2>"
using namespace std;
/********************/

int main(int argc, char* argv[]){
    if(argc == 2 && std::string(argv[1]) == "-h"){
        cout << endl;
        cout << filename << " :: " << description << endl;
        cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
        cout << "Usage: ./bin/" << filename << " " << arg_desc << endl;
        cout << "Trigger mode 0 = sw, 1 = hardware, 2 = scalar mode" << endl;
        return 1;
    }
    else if(argc > NUM_ARGS+1 || argc < NUM_ARGS){
        cout << "error: too many number of arguments" << endl;
        cout << "Usage: ./bin/" << filename << " " << arg_desc << endl;
        return -1;
    }
    else{
        int num_checks = 5;
        int num_events = 100;
        SuMo command;
        char log_data_filename[1300];

        strcpy(log_data_filename, argv[1]);
        num_events = atoi(argv[2]);
        int trig_mode = atoi(argv[3]);

        if(command.check_active_boards(num_checks))
            return 1;


        return command.log_data(num_events, trig_mode, 0, log_data_filename);
    }
}