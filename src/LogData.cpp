#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"

using namespace std;

int main(int argc, char* argv[]){

  SuMo command;
  char log_data_filename[100];
 
  sprintf(log_data_filename, "%s", argv[1]);
  int num = 100;
  num = atoi(argv[2]);
  int num2 = atoi(argv[3]);

  command.read_CC(false, false);
  command.read_CC(true, false);

  usleep(1000);
  command.log_data(log_data_filename, num, num2, 10000);

  return 0;
}
