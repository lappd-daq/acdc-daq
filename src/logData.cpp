#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
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

static int LIMIT_READOUT_RATE  = 6000;  //usecs limit between event polling
static int NUM_SEQ_TIMEOUTS    = 100;    // number of sequential timeouts before ending run
const  float  MAX_INT_TIMER    = 800.;    // max cpu timer before ending run (secs)
/* note: usb timeout defined in include/stdUSB.h */

bool overwriteExistingFile = false;

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
    SuMo command;
    char log_data_filename[100];
    
    strcpy(log_data_filename, argv[1]);
    num_events = atoi(argv[2]);
    int trig_mode = atoi(argv[3]);
    
    if(command.check_active_boards(num_checks))
      return 1;

    command.log_data(log_data_filename, num_events, trig_mode, 0);
    
    return 0;
  }
}

int SuMo::log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate){
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

  /* handle filename */
  // 'scalar' mode
  ofstream rate_fs;
  if(trig_mode == 2){
    char logRateFilename[300];
    sprintf(logRateFilename, "%s.acdc.rate", log_filename);
    rate_fs.open(logRateFilename, ios::trunc);
  }
  
  // full waveform, standard mode
  time(&now);
  char timestring[100];
  strftime(timestring, 80, "%Y-%m-%d-%H-%M", localtime(&now));  
  //sprintf(logDataFilename, "%s-%s.acdc.dat", timestring, log_filename);
  sprintf(logDataFilename, "%s.acdc.dat", log_filename);

  // check if file exists, inquire whether to overwrite 
  // shouldn't be an issue now since file timestamped in filename ^^
  string temp;
  while(fileExists(logDataFilename)){
    cout << "file already exists, try new filename: (or enter to overwrite / ctrl-C to quit): ";
    getline(cin, temp);
    if(temp.empty()) break;
    sprintf(logDataFilename, "%s.acdc.dat", temp.c_str());
  }
  
  ofstream ofs;
  ofs.open(logDataFilename, ios::trunc);

  // read all front end cards 
  bool all[numFrontBoards];
  for(int i=0;i<numFrontBoards; i++) all[i] = true;

  load_ped();

  int number_of_frontend_cards = 0;
  // save pedestal data to file header for reference, easy access 
  // zero pad to match data format 
  for(int i=0; i < psecSampleCells; i++){
    
    ofs << i << " " << 0 << " ";
    for(int board=0; board<numFrontBoards; board++){
      if(DC_ACTIVE[board]){
	if(i == 0) number_of_frontend_cards++;
	for(int channel=0; channel < AC_CHANNELS; channel++) ofs << PED_DATA[board][channel][i]<< " "; 
	ofs << 0 << " ";
      }
    }
    ofs << endl;
  }

  cout << "--------------------------------------------------------------" << endl;
  cout << "number of front-end boards detected = " << number_of_frontend_cards 
       << " of " << numFrontBoards << " address slots in system" << endl;
  cout << "Trying for " << NUM_READS << " events logged to disk, in a timeout window of "
       << MAX_INT_TIMER << " seconds" << endl;
  cout << "--------------------------------------------------------------" << endl << endl;

  usleep(100000);

  /* cpu time zero */
  //t0 = time(NULL);
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
	  adcDat[targetAC]->Data[AC_CHANNELS][j] = Meta[j];
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
  
  cout << endl;
  cout << "Done on readout:  " << last_k+1 << " :: @time " <<t<< " sec" << endl;
   
  cleanup();
 
  /* add whitespace to end of file */
  ofs <<endl<<endl;
  ofs.close();

  if(trig_mode == 2) rate_fs.close();

  dump_data();

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





