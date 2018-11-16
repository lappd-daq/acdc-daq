/////////////////////////////////
// created: 31-10-2014
// AC/DC system software
// setPed.cpp
// written: ejo
/////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

/* specific to file */
const int NUM_ARGS  =    2;
const char* filename =     "setPed";
const char* description=   "set voltage pedestal on front-end";

using namespace std;

int main(int argc, char* argv[]){
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1;
  }
  else if(argc != NUM_ARGS){
    cout << "error: wrong number of arguments" << endl;
    return -1;
  }
  /* function defined below */
  else{
    SuMo Sumo;
    int temp = 0;
    int num_checks = 3;
    int device = 0;
    int ped = atoi(argv[1]);
    if(ped > 4095 || ped < 0){
      cout << "invalid pedestal value" << endl;
      return 1;
    }
    Sumo.set_usb_read_mode(16);
    if(Sumo.check_active_boards(num_checks))
      return 1;

    Sumo.set_pedestal_value(ped,15, device );
 
    int mode = Sumo.check_readout_mode();
    if(mode == 1 && Sumo.check_active_boards_slaveDevice() > 0){
      cout << "Slave board detected " << endl;
      device = 1;
      Sumo.set_pedestal_value(ped, 15, device);

    }

    cout << "Pedestal Voltage set to " << ped*1200/4096 << " mV\n";

    return 0;
  }
}
