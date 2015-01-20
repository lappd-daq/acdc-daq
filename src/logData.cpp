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
/********************/

/* subtract pedestal values on-line */
static bool PED_SUBTRCT = false; 

static int LIMIT_READOUT_RATE = 10000;
static int NUM_SEQ_TIMEOUTS = 100;
const  float  MAX_INT_TIMER    = 600.;  // 10 minutes
/* note: usb timeout defined in include/stdUSB.h */

bool overwriteExistingFile = false;

bool fileExists(const std::string& filename);

int main(int argc, char* argv[]){
  if(argc == 2 && std::string(argv[1]) == "-h"){
    cout << endl;
    cout << filename << " :: " << description << endl;
    cout << filename << " :: takes " << NUM_ARGS-1 << " arguments" << endl;
    return 1; 
  }
  else if(argc > NUM_ARGS+1 || argc < NUM_ARGS){
    cout << "error: too many number of arguments" << endl;
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
    
    if(command.check_active_boards(num_checks))
      return 1;

    command.log_data(log_data_filename, num_events, trig_mode, acq_rate);
    
    return 0;
  }
}

int SuMo::log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate){
  bool convert_to_voltage;
  int sample, check_event, asic_baseline[psecSampleCells], count = 0, psec_cnt = 0, numTimeouts = 0, last_k;
  float  _now_, t = 0.;
  char logDataFilename[300];
  Timer timer = Timer(); 
  time_t now;

  /* handle filename */
  // 'scalar' mode
  ofstream rate_fs;
  if(trig_mode == 2){
    char logRateFilename[300];
    sprintf(logRateFilename, "%s.acdc.rate", log_filename);
    rate_fs.open(logRateFilename, ios::trunc);
  }
  
  // full waveform, standard mode
  sprintf(logDataFilename, "%s.acdc.dat", log_filename);
  string temp;
  while(fileExists(logDataFilename)){
    cout << "file already exists, try new filename: (or enter to overwrite / ctrl-C to quit): ";
    getline(cin, temp);
    if(temp.empty()) break;
    sprintf(logDataFilename, "%s.acdc.dat", temp.c_str());
  }
  
  ofstream ofs;
  ofs.open(logDataFilename, ios::trunc);

  /* read all front end cards */
  bool all[numFrontBoards];
  for(int i=0;i<numFrontBoards; i++) all[i] = true;

  /* setup some things */
  if(trig_mode==1) {
                    set_usb_read_mode(24);
    if(mode==USB2x) set_usb_read_mode_slaveDevice(24);
  }
  else {
    if(mode==USB2x) set_usb_read_mode_slaveDevice(16);
    set_usb_read_mode(16), dump_data();
  } 
   
  load_ped();
  /* save pedestal data to file header for reference, easy access */
  /* zero pad to match data format */
  for(int i=0; i < psecSampleCells; i++){
    
    ofs << i << " " << 0 << " ";
    for(int board=0; board<numFrontBoards; board++){
      if(DC_ACTIVE[board]){
	for(int channel=0; channel < AC_CHANNELS; channel++) ofs << PED_DATA[board][channel][i]<< " "; 
	ofs << 0 << " ";
      }
    }
    ofs << endl;
  }

  /* cpu time zero */
  //t0 = time(NULL);
  timer.start();

  for(int k=0;k<NUM_READS; k++){
    /* unix timestamp */
    time(&now);
    /* rough cpu timing in seconds since readout start time*/
    t = timer.stop(); 
    if(t > MAX_INT_TIMER) {
      cout << endl << "readout timed out at " << t << "on event " << k << endl;
      break;
    }
    /*set read mode to NULL */
    set_usb_read_mode(16);
    if(mode==USB2x) set_usb_read_mode_slaveDevice(16);
    /*reset last event on firmware */
    manage_cc_fifo(1);
    if(mode==USB2x) manage_cc_fifo_slaveDevice(1);

    /*send trigger over software if not looking externally */
    if(trig_mode==1){ 
                         reset_self_trigger(15, 0), set_usb_read_mode(7);
      if(mode == USB2x)  reset_self_trigger(15, 1), set_usb_read_mode_slaveDevice(7);
      usleep(acq_rate+LIMIT_READOUT_RATE);
                         set_usb_read_mode(0);
      if(mode == USB2x)  set_usb_read_mode_slaveDevice(0);
    }
    else{
      // 'rate-only' mode, only pull data every second
      if(trig_mode == 2){
	usleep(3e6);
      }
      
      software_trigger((unsigned int)15);
      if(mode == USB2x) software_trigger_slaveDevice((unsigned int)15);
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
    // timeout on all boards
    if( numBoards == 0 ){
      ofs << k << " " << 0xFF << " " << endl;

      k = k-1;  //repeat event
      continue;	
      
    }
    /*
    int numBoardsTimedout = 0;
    for(int i=0; i<numFrontBoards; i++) numBoardsTimedout = numBoardsTimedout + (int)BOARDS_TIMEOUT[i];
    if(numBoards == 0 || numBoardsTimedout > 0){
      // check for number of timeouts in a row 
      numTimeouts++; 
      // handle if too many timeouts: 
      if(numTimeouts > NUM_SEQ_TIMEOUTS){
	cout << endl << "error: too many timeouts in a row" << endl; 
	return -1;
      }
      k = k-1;
      continue; 
    }
    else numTimeouts = 0; 
    */

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
	    sample = adcDat[targetAC]->AC_RAW_DATA[psec_cnt][i%6*256+j];
	    if(PED_SUBTRCT) sample -= PED_DATA[targetAC][i][j];          
	    
	    adcDat[targetAC]->Data[i][j] = sample;
	  }
	}  
	/* wraparound_correction, if desired: */
	int baseline[psecSampleCells];
	unwrap_baseline(baseline, 2); 
	for (int j = 0; j < psecSampleCells; j++) asic_baseline[j] = baseline[j];
      }
      //if timeout on only some, but not all boards
      else if( numBoards > 0 && BOARDS_TIMEOUT[targetAC] ){
	for(int i = 0; i < AC_CHANNELS; i++){
	  if(i>0 && i % 6 == 0) psec_cnt ++;

	  for(int j = 0; j < psecSampleCells; j++){
	    sample = 0xFF;            
	    adcDat[targetAC]->Data[i][j] = sample;
	  }
	}  
      }
    }
	
    for(int i=0; i < psecSampleCells; i++){

      ofs << i << " " << asic_baseline[i] << " ";
      for(int board=0; board<numFrontBoards; board++)
	if(BOARDS_READOUT[board])
	  for(int channel=0; channel < AC_CHANNELS+1; channel++) ofs << std::dec << adcDat[board]->Data[channel][i] << " ";
        else if(BOARDS_TIMEOUT[board])
  	  for(int channel=0; channel < AC_CHANNELS+1; channel++) ofs << std::dec << adcDat[board]->Data[channel][i] << " ";

      ofs <<endl;
    } 
  
    if(trig_mode == 2){
      for(int board=0; board<numFrontBoards; board++){
	if(BOARDS_READOUT[board]){

	  rate_fs << k << "\t" << board << "\t" << t << "\t";

	  for(int channel=0; channel < AC_CHANNELS; channel++)  rate_fs <<  adcDat[board]->self_trig_scalar[channel] << "\t";
	  
	  rate_fs << endl;
	}
      }
    }
    last_k = k;
  }
  
  cout << "Readout:  " << last_k+1<< " of " << NUM_READS << " :: @time " <<t<< " sec...........\r", usleep(100000),cout.flush();
  cout << "Readout:  " << last_k+1 << " of " << NUM_READS << " :: @time " <<t<< " sec...........finished logging\r",cout.flush();

  manage_cc_fifo(1);
  /* add whitespace to end of file */
  ofs <<endl<<endl;
  ofs.close();

  if(trig_mode == 2) rate_fs.close();

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
  adcDat[i]->Data[AC_CHANNELS][0] = count;
  adcDat[i]->Data[AC_CHANNELS][1] = Address;
  adcDat[i]->Data[AC_CHANNELS][2] = adcDat[i]->CC_EVENT_COUNT;
  adcDat[i]->Data[AC_CHANNELS][3] = adcDat[i]->CC_BIN_COUNT; 
  adcDat[i]->Data[AC_CHANNELS][4] = adcDat[i]->timestamp_hi;
  adcDat[i]->Data[AC_CHANNELS][5] = adcDat[i]->timestamp_mid;
  adcDat[i]->Data[AC_CHANNELS][6] = adcDat[i]->timestamp_lo;
  
  adcDat[i]->Data[AC_CHANNELS][7] = adcDat[i]->CC_TIMESTAMP_HI;
  adcDat[i]->Data[AC_CHANNELS][8] = adcDat[i]->CC_TIMESTAMP_MID;
  adcDat[i]->Data[AC_CHANNELS][9] = adcDat[i]->CC_TIMESTAMP_LO;

  adcDat[i]->Data[AC_CHANNELS][10] = adcDat[i]->bin_count_rise;
  adcDat[i]->Data[AC_CHANNELS][11] = adcDat[i]->bin_count_fall;
  adcDat[i]->Data[AC_CHANNELS][12] = adcDat[i]->self_trig_settings; 
  adcDat[i]->Data[AC_CHANNELS][13] = adcDat[i]->event_count;
  adcDat[i]->Data[AC_CHANNELS][14] = adcDat[i]->reg_self_trig[0];
  adcDat[i]->Data[AC_CHANNELS][15] = adcDat[i]->reg_self_trig[1]; 
  adcDat[i]->Data[AC_CHANNELS][16] = adcDat[i]->reg_self_trig[2];
  adcDat[i]->Data[AC_CHANNELS][17] = adcDat[i]->reg_self_trig[3]; 
  adcDat[i]->Data[AC_CHANNELS][18] = adcDat[i]->self_trig_mask; 
  adcDat[i]->Data[AC_CHANNELS][19] = adcDat[i]->last_ac_instruct; 
  adcDat[i]->Data[AC_CHANNELS][20] = adcDat[i]->last_last_ac_instruct;
  adcDat[i]->Data[AC_CHANNELS][21] = adcDat[i]->trig_en; 
  adcDat[i]->Data[AC_CHANNELS][22] = adcDat[i]->trig_wait_for_sys; 
  adcDat[i]->Data[AC_CHANNELS][23] = adcDat[i]->trig_rate_only; 
  adcDat[i]->Data[AC_CHANNELS][24] = adcDat[i]->trig_sign; 

  /*fill slots 25-39 */
  for(int j=0; j<numChipsOnBoard; j++){
    adcDat[i]->Data[AC_CHANNELS][j+25] = adcDat[i]->ro_cnt[j]; 
    adcDat[i]->Data[AC_CHANNELS][j+25+numChipsOnBoard] = adcDat[i]->vbias[j]; 
    adcDat[i]->Data[AC_CHANNELS][j+25+2*numChipsOnBoard] = adcDat[i]->trigger_threshold[j]; 
  }
  /*fill slots 40-69 */
  for(int j=0; j<AC_CHANNELS; j++){
    adcDat[i]->Data[AC_CHANNELS][j+40] = adcDat[i]->self_trig_scalar[j];
  }
  adcDat[i]->Data[AC_CHANNELS][70] = cpuTime;
  adcDat[i]->Data[AC_CHANNELS][71] = now;
  adcDat[i]->Data[AC_CHANNELS][72] = WRAP_CONSTANT; 

}



