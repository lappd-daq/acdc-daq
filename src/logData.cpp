#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <sys/stat.h>
#include <time.h>

#include "SuMo.h"
#include "Timer.h"

/* specific to file */
const int NUM_ARGS = 4;
const char* filename = "logData";
const char* description = "log data from DAQ";
using namespace std;
  
static int LIMIT_READOUT_RATE = 10000;
static int NUM_SEQ_TIMEOUTS = 100;
/* note: usb timeout defined in include/stdUSB.h */

bool fileExists(const std::string& filename);

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
  else{
    int num_checks = 5; 
    int num_events = 100;
    int acq_rate   = 10000;
    SuMo command;
    char log_data_filename[100];
    
    strcpy(log_data_filename, argv[1]);
    num_events = atoi(argv[2]);
    int trig_mode = atoi(argv[3]);
    
    command.set_usb_read_mode(16); 
    if(command.check_active_boards(num_checks))
      return 1;

    command.log_data(log_data_filename, num_events, trig_mode, acq_rate);
    
    return 0;
  }
}

int SuMo::log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate){
  bool convert_to_voltage;
  int check_event, asic_baseline[psecSampleCells], count = 0, psec_cnt = 0, numTimeouts = 0;
  float sample, _now_, t = 0.;
  char logDataFilename[300];
  Timer timer = Timer(); 
  time_t now;

  /* handle filename */
  sprintf(logDataFilename, "%s.txt", log_filename);
  string temp;
  while(fileExists(logDataFilename)){
    cout << "file already exists, try new filename: (or enter to overwrite): ";
    getline(cin, temp);
    if(temp.empty()) break;
    sprintf(logDataFilename, "%s.txt", temp.c_str());
  }
  ofstream ofs;
  ofs.open(logDataFilename, ios::trunc);

  /* read all front end cards */
  bool all[numFrontBoards];
  for(int i=0;i<numFrontBoards; i++) all[i] = true;
  
  /* need to add voltage LUT stuff here, eventually */
  convert_to_voltage = false;
  /*if(load_lut() != 0) convert_to_voltage = false;
   *else
   *  convert_to_voltage = output_mode;
   */

  /* setup some things */
  if(trig_mode) set_usb_read_mode(24);
  else set_usb_read_mode(16), dump_data();
  
  load_ped();
  /* cpu time zero */
  //t0 = time(NULL);
  timer.start();

  for(int k=0;k<NUM_READS; k++){
    /* unix timestamp */
    time(&now);
    /* rough cpu timing in seconds since readout start time*/
    t = timer.stop(); 
    /*set read mode to NULL */
    set_usb_read_mode(16);
    /*reset last event on firmware */
    manage_cc_fifo(1);
    
    /*send trigger over software if not looking externally */
    if(trig_mode){ 
      set_usb_read_mode(7);
      usleep(acq_rate+LIMIT_READOUT_RATE);
    }
    else{
      software_trigger((unsigned int)15);
      usleep(LIMIT_READOUT_RATE); /* somewhat arbitrary hard-coded rate limitation */
    }

    /* show event number at terminal */
    if((k+1) % 2 == 0 || k==0){
      cout << "Readout:  " << k+1 << " of " << NUM_READS << " :: @time "<< t << " sec         \r";
      cout.flush();
    }       
    /**************************************/
    /*Do bulk read on all front-end cards */  
    int numBoards = read_AC(1, all, false);
    /**************************************/
    /* handle timeouts or lack of data */
    int numBoardsTimedout = 0;
    for(int i=0; i<numFrontBoards; i++) numBoardsTimedout = numBoardsTimedout + (int)BOARDS_TIMEOUT[i];
    if(numBoards == 0 || numBoardsTimedout > 0){
      /* check for number of timeouts in a row */
      numTimeouts++; 
      /* handle if too many timeouts: */
      if(numTimeouts > NUM_SEQ_TIMEOUTS){
	cout << endl << "error: too many timeouts in a row" << endl; 
	return -1;
      }
      k = k-1;
      continue; 
    }
    else numTimeouts = 0;

    /* form data for filesave */
    for(int targetAC = 0; targetAC < numFrontBoards; targetAC++){ 
      if(BOARDS_READOUT[targetAC] && numBoards > 0){
	psec_cnt = 0;
	/*assign meta data */
	get_AC_info(false, targetAC);
	form_meta_data(targetAC, k, t, now);

	check_event = 0;

	for(int i = 0; i < AC_CHANNELS; i++){
	  if(i>0 && i % 6 == 0) psec_cnt ++;

	  for(int j = 0; j < psecSampleCells; j++){
	    if(convert_to_voltage){
	      //sample =  LUT[(int)AC_RAW_DATA[psec_cnt][i%6*256+j]][i]*1000;
	      //sample -= LUT[(int)PED_DATA[targetAC][i][j]][i]*1000;
	    }
	    else{
	      sample = (float) acdcData[targetAC].AC_RAW_DATA[psec_cnt][i%6*256+j];
	      sample -= (float) PED_DATA[targetAC][i][j];
	    }
	    acdcData[targetAC].Data[i][j] = sample;
	  }
	}  
	/* wraparound_correction, if desired: */
	int baseline[psecSampleCells];
	unwrap_baseline(baseline, 2); 
	for (int j = 0; j < psecSampleCells; j++) asic_baseline[j] = baseline[j];
      }
    }
	
    for(int i=0; i < psecSampleCells; i++){

      ofs << i << " " << asic_baseline[i] << " ";
      for(int board=0; board<numFrontBoards; board++)
	if(BOARDS_READOUT[board])
	  for(int channel=0; channel < AC_CHANNELS+1; channel++) ofs << std::dec << acdcData[board].Data[channel][i] << " ";
  
      ofs <<endl;
    } 
  }
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec.\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec..\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec...\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec....\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec.....\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec......\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec.......\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec........\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec.........\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec..........\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec...........\r", usleep(100000),cout.flush();
  cout << "Readout:  " << NUM_READS << " of " << NUM_READS << " :: @time " <<t<< " sec...........finished logging\r",cout.flush();

  manage_cc_fifo(1);
  /* add whitespace to end of file */
  ofs <<endl<<endl;
  ofs.close();

  set_usb_read_mode(16);  //turn off trigger, if on
  dump_data();
  
  cout << endl;
  cout << "Data saved in file: " << logDataFilename << endl << "*****" << endl;
  return 0;
}

bool fileExists(const string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}

void SuMo::form_meta_data(int Address, int count, double cpuTime, time_t now)
{
  int i = Address;
  acdcData[i].Data[AC_CHANNELS][0] = count;
  acdcData[i].Data[AC_CHANNELS][1] = acdcData[i].CC_EVENT_COUNT;
  acdcData[i].Data[AC_CHANNELS][2] = acdcData[i].CC_BIN_COUNT; 
  acdcData[i].Data[AC_CHANNELS][3] = acdcData[i].EVENT_COUNT;
  acdcData[i].Data[AC_CHANNELS][4] = acdcData[i].TIMESTAMP_HI;
  acdcData[i].Data[AC_CHANNELS][5] = acdcData[i].TIMESTAMP_MID;
  acdcData[i].Data[AC_CHANNELS][6] = acdcData[i].TIMESTAMP_LO;
  
  acdcData[i].Data[AC_CHANNELS][7] = acdcData[i].CC_TIMESTAMP_HI;
  acdcData[i].Data[AC_CHANNELS][8] = acdcData[i].CC_TIMESTAMP_MID;
  acdcData[i].Data[AC_CHANNELS][9] = acdcData[i].CC_TIMESTAMP_LO;

  acdcData[i].Data[AC_CHANNELS][10] = acdcData[i].BIN_COUNT_RISE;
  acdcData[i].Data[AC_CHANNELS][11] = acdcData[i].BIN_COUNT_FALL;
  acdcData[i].Data[AC_CHANNELS][12] = acdcData[i].SELF_TRIG_SETTINGS; 
  acdcData[i].Data[AC_CHANNELS][13] = acdcData[i].EVENT_COUNT ;
  acdcData[i].Data[AC_CHANNELS][14] = acdcData[i].REG_SELF_TRIG[0];
  acdcData[i].Data[AC_CHANNELS][15] = acdcData[i].REG_SELF_TRIG[1]; 
  acdcData[i].Data[AC_CHANNELS][16] = acdcData[i].REG_SELF_TRIG[2];
  acdcData[i].Data[AC_CHANNELS][17] = acdcData[i].REG_SELF_TRIG[3]; 
  acdcData[i].Data[AC_CHANNELS][18] = acdcData[i].SELF_TRIG_MASK; 
  acdcData[i].Data[AC_CHANNELS][19] = acdcData[i].LAST_AC_INSTRUCT; 
  acdcData[i].Data[AC_CHANNELS][20] = acdcData[i].LAST_LAST_AC_INSTRUCT;
  acdcData[i].Data[AC_CHANNELS][21] = acdcData[i].TRIG_EN; 
  acdcData[i].Data[AC_CHANNELS][22] = acdcData[i].TRIG_WAIT_FOR_SYS; 
  acdcData[i].Data[AC_CHANNELS][23] = acdcData[i].TRIG_RATE_ONLY; 
  acdcData[i].Data[AC_CHANNELS][24] = acdcData[i].TRIG_SIGN; 

  /*fill slots 25-39 */
  for(int j=0; j<numChipsOnBoard; j++){
    acdcData[i].Data[AC_CHANNELS][j+25] = acdcData[i].RO_CNT[j]; 
    acdcData[i].Data[AC_CHANNELS][j+25+numChipsOnBoard] = acdcData[i].VBIAS[j]; 
    acdcData[i].Data[AC_CHANNELS][j+25+2*numChipsOnBoard] = acdcData[i].TRIGGER_THRESHOLD[j]; 
  }
  /*fill slots 40-69 */
  for(int j=0; j<AC_CHANNELS; j++){
    acdcData[i].Data[AC_CHANNELS][j+40] = acdcData[i].SELF_TRIG_SCALER[j];
  }
  acdcData[i].Data[AC_CHANNELS][70] = cpuTime;
  acdcData[i].Data[AC_CHANNELS][71] = now;
  acdcData[i].Data[AC_CHANNELS][72] = WRAP_CONSTANT; 

}



