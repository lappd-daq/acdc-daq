#include <string.h>
#include <strings.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "SuMo.h"
using namespace std;
//Measure pulse-triggering turn on curves for each channel on an ACDC board.
//takes pedestal (DAC counts), board number, pulse amplitude (mV), savefile name, and channel mask (optional hex word) as input.
//if no channel mask is given, all channels are tested by default.
//John Podczerwinski and John Judge
//May 2018

int hexMaskToint(char* hex) {
  char* fixed = hex;
  int len = strlen(fixed);
  if ((hex[0] != '0') && (hex[1] != 'x')) {
    fixed = hex + 2;
  }
  unsigned long int imask = strtoul(hex, NULL, 16);
  if (imask == 0) {exit(1);}
  if(imask > 0x3FFFFFFF) {
    printf("entered channel mask %s > 3FFFFFFF (only have 30 channels)\n", fixed);
    exit(1);
  }
  return imask;
}


int main(int argc,char* argv[])
{
  if (argc != 5 and argc != 6){
    printf("expecting 4 or 5 arguments: pedestal (in DAC counts), board number, Pulse Amplitude (mV), Savefile, and channel mask (optional 30 bit binary sequence)\n");
	    exit(0);
  }
    //Declaring Variables. ****************************************************
    SuMo sumo; //creating an instance of SuMo
    int pedestal = atoi(argv[1]); //Assigning a variable to the user's input.
    int board_number = atoi(argv[2]); //same deal.
    int amplitude = floor(atoi(argv[3])*4096/1000);
    char* filename = argv[4];
    bool chan_mask[30];

    if (argc == 6){
      int mask = hexMaskToint(argv[5]);
      for (int K = 0; K < 30; K++){
	chan_mask[K] = !!(mask & (1<<K)); //select Kth binary digit
      }
    }
    else {
      for (int L = 0; L < 30; L++){
	chan_mask[L] = 1;
      }
    }
    cout << filename << endl;

    printf("Channels to test: {");
    int comma = 0;
    for (int K = 0; K<30;K++) {
      if(chan_mask[K]) {
	if(comma) {cout << ", ";}
	cout << (K+1);
	comma = 1;
      }
    }
    printf("}\n");
    
    unsigned int sent_word = 0x00000000;
    unsigned int trig_sel = 1;
    unsigned int trig_mask = 0x00000000;
    unsigned int boardAddress = pow(2,board_number); 
    bool trig_enable = true; //Allow self trigs
    bool wait_for_sys = false; //don't wait for sys.
    bool rate_only = true; //put in scalar mode
    bool trig_sign = false; //trigger on falling edge.
    bool sma_trig_on_fe = false; //don't trigger on sma.
    bool use_coinc = false; //don't use coincidences for triggering.
    bool use_trig_valid = false; //don't use trig valid to reset ACDC trig state machines.
    unsigned int coinc_pulsew = 7; //all of these coinc related things don't matter since use_coinc = 0.
    unsigned int coinc_window = 7; //setting them to 7 to be consistent with Eric's setconfig code.
    unsigned int coinc_num_asic = 7;
    unsigned int coinc_numch = 7; 
    int device = 0; //no slaves
    unsigned int chipAddress; //This will come into play later on.
    bool AC_read_mask[8]; //read mask used for reading out ACDC boards. Using 8 since the ACC is designed for 8 boards.
    int counts = 0; //Number of counts reported by scalar mode.
    int low_thresh = 0; //A value that will be used in the loop coming up.
    int num_boards = 0; //Used to keep track of communication.
    ofstream rate_fs;
    int failure = 0; //count the number of times communication cuts out. If it does it a certain number of times, give up.

    char savefile[300];
    sprintf(savefile,"%s.dat",filename); //create the name of your savefile
    rate_fs.open(savefile,ios::trunc);
    for (int i = 0;i<8;i++) AC_read_mask[i] = true; //Initializing the read mask.
    int trig_counts_save = 0;
    int noise_counts_save = 0;
    int noise_counts = 0;
    int trig_counts = 0;
    //***************************************************************************
    //**************************************************************************
    
    
    
    //Initializing Pedestal and Triggering options.
    sumo.set_usb_read_mode(16); //Allows ACC to pass messages to ACDC.
    sumo.set_pedestal_value(pedestal,15,0); //setting the pedestal.
    sumo.set_self_trigger_lo(trig_enable,wait_for_sys,rate_only,trig_sign,sma_trig_on_fe,use_coinc,use_trig_valid,coinc_window,boardAddress,0); //send the first self trigger setting word.
    usleep(100); //wait 100usec
    sumo.set_self_trigger_hi(coinc_pulsew,coinc_num_asic,coinc_numch,boardAddress,0); //send the second trigger word.
    //****************************************************************************
    //***************************************************************************
    int lower_lim;
    if (2*amplitude > 100){ //make sure the offset effect doesn't cause you to miss anything.
      lower_lim = pedestal-2*amplitude;
    }
    else {
      lower_lim = pedestal - 100;
    }
    for (int i = 0; i<30; i++){ //for each channel on the board
      //Adjusting the trigger mask
      if (chan_mask[i] == 1){
	trig_mask = sent_word| trig_sel << i; //creates new trig mask to select channel.
	sumo.set_usb_read_mode(16); //make sure that the ACDC will receieve these messages
	sumo.set_self_trigger_mask(0x00007FFF&trig_mask, 0, boardAddress, 0); //Adjust self trigger mask, first word.
	sumo.set_self_trigger_mask((0x3FFF8000&trig_mask) >> 15, 1, boardAddress, 0); //Adjust self trigger mask, second word.
	rate_fs << i+1 << endl;
	noise_counts_save = 0; //make sure it's 0 so you don't prematurely exit loop.
	if (lower_lim < 0) {lower_lim = 0;}
	int upper_lim = pedestal + 100;
	if(upper_lim > 4095) {upper_lim = 4095;}
	if((pedestal > 4195) || (pedestal < 0)) {printf("pedestal out of range\n"); exit(1);}
	for (int j = lower_lim; j<upper_lim;){ //for thresholds from min to max...
	  
	  sumo.adjust_thresh(j,board_number); //adjust the threshold for all chips on the ACDC board.
	  num_boards = sumo.measure_rate(AC_read_mask); //Measure the self trigger rate for the board in question.
	  sumo.get_AC_info(false,board_number);
	  sumo.toggle_CAL(true,device);
	  if (sumo.adcDat[board_number]->vbias[3] == 0){
	    cout << "communication has cut out at channel " << i+1 << " and thresh " << j << endl;
	    rate_fs.close();
	    return 1;
	  }
	  else if  (num_boards < 1){ //check and make sure that communication between the ACC and ACDC has not cut out. 
	    cout << " minor communication cut out at channel " << i+1 << " and thresh " << j <<  endl; //Give you the heads up that it didn't work.
	  }
	  
	  else {
	    cout << "Channel " << i+1 << " THRESH " << j << " trigger rate " << sumo.adcDat[board_number]->self_trig_scalar[i] << endl; 
	    if (sumo.adcDat[board_number] -> self_trig_scalar[i] > 5){
	      low_thresh = j;
	      int l = low_thresh-6;
	      noise_counts_save = 0;
	      while(l < pedestal+100 and (l < low_thresh+20 or trig_counts_save > 5)){ //go from here until we hit noise.
		sumo.adjust_thresh(l,board_number); //Adjust the threshold.
		usleep(1e6); //sleep for a microsecond.
		sumo.toggle_CAL(false,device);
		for (int z=0;z<5;z++){ //repeat 5 times...
		  sumo.measure_rate(AC_read_mask); //gather some data
		  noise_counts = noise_counts+sumo.adcDat[board_number]->self_trig_scalar[i]; //add that on to noise counts.
		}
		sumo.toggle_CAL(true,device);
		for (int y=0;y<5;y++){
		  sumo.measure_rate(AC_read_mask);
		  trig_counts = trig_counts+sumo.adcDat[board_number]->self_trig_scalar[i];
		}
		
		// cout << "THRESH " << l << " noise " << sumo.adcDat[board_number]->self_trig_scalar[i] << endl;
		rate_fs  << l << "\t" << noise_counts << "\t" << trig_counts << endl;
		l = l+1;
		cout << " Threshold  " << l-1 << " noise_counts " << noise_counts << " trig_counts " << trig_counts << endl;
		trig_counts_save = trig_counts;
		noise_counts_save = noise_counts;
		noise_counts = 0;
		trig_counts = 0;
		if (noise_counts_save > 1000) break; //exit the while loop if we hit the noise ceiling..
	      }
	      low_thresh = 0;
	      j = l;
	      
	    }
	    
	    else j = j+5;
	  }
	  if (noise_counts_save > 1000) break; //exit the threshold scanning loop when we hit the noise ceiling.
	}
      }
    }
      

         
  
	  

    rate_fs.close();//
    sumo.dump_data();
	

    
    return 0;
    
}

