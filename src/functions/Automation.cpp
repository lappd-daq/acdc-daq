#include <string>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <bitset>
#include "SuMo.h"
using namespace std;
//Measure Noise Triggering Turn on Curves for the channels on an ACDC board.
//Takes Pedestal (DAC counts), board number, savefile name, and channel mask (optional argument, entered in hex) as inputs.
//If no channel mask is given, all channels are tested by default.
//John Podczerwinski and John Judge//May 2018

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
  if (argc != 4 and argc != 5){
    printf("expecting 3 or 4 arguments: (pedestal) in DAC counts, (board number), save filename, and channel mask (optional hex word)\n");
	    exit(0);
  }
    //Declaring Variables. ****************************************************
    SuMo sumo; //creating an instance of SuMo
    int pedestal = atoi(argv[1]); //Assigning a variable to the user's input.
    int board_number = atoi(argv[2]); //same deal.
    char* filename = argv[3];
    bool chan_mask[30];
    if (argc == 5){
      int mask = hexMaskToint(argv[4]);
      //if the user requests a specific mask, use it.
      int chan_dig = 0;
      for (int K = 0; K < 30; K++) {
        chan_dig = !!(mask & (1<<K)); //selects out the Kth binary digit of mask
	chan_mask[K] = chan_dig; 
      
      }
    
    }
    else{
      //otherwise, set chan mask to take measurement on every channel.
      for (int L = 0; L < 30; L++){
	chan_mask[L] = 1;
      }
    }
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
    sprintf(savefile,"%s.dat",filename);
    rate_fs.open(savefile,ios::trunc);
    for (int i = 0;i<8;i++) AC_read_mask[i] = true; //Initializing the read mask.
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

    for (int i = 0; i<30; i++){ //for each channel on the board
      //Adjusting the trigger mask
      
      if (chan_mask[i]){
	
	trig_mask = sent_word| trig_sel << i; //creates new trig mask to select channel.
	sumo.set_usb_read_mode(16); //make sure that the ACDC will receieve these messages
	sumo.set_self_trigger_mask(0x00007FFF&trig_mask, 0, boardAddress, 0); //Adjust self trigger mask, first word.
	sumo.set_self_trigger_mask((0x3FFF8000&trig_mask) >> 15, 1, boardAddress, 0); //Adjust self trigger mask, second word.
	rate_fs << "\t" << i+1 << endl;
	if((pedestal < 0) || (pedestal > 4595)) {
	  printf("pedestal out of range \n"); exit(1);
	}
	int lowt = pedestal-500;
	if (lowt<0) {lowt = 0;};
	int hight = pedestal + 500;
	if (hight>4095) {hight = 4095;}

	for (int j = lowt; j<hight;){ //for thresholds from min to max...
	   
	  sumo.adjust_thresh(j,board_number); //adjust the threshold for all chips on the ACDC board.
	  num_boards = sumo.measure_rate(AC_read_mask); //Measure the self trigger rate for the board in question.
	  sumo.get_AC_info(false,board_number);
	  if (sumo.adcDat[board_number]->vbias[3] == 0){
	    cout << "communication has cut out at channel " << i+1 << " and thresh " << j << endl;
	    rate_fs.close();
	    return 1;
	  }
	  else if  (num_boards < 1){ //check and make sure that communication between the ACC and ACDC has not cut out. 
	    cout << " minor communication cut out at channel " << i+1 << " and thresh " << j <<  endl; //Give you the heads up that it didn't work.
	  }
	  
	  else {
	    cout << "Channel " << i+1 << " THRESH " << j << " noise " << sumo.adcDat[board_number]->self_trig_scalar[i] << endl; 
	    if (sumo.adcDat[board_number] -> self_trig_scalar[i] > 5){ //if we noise rates greater than 5Hz...
	      low_thresh = j; 
	      int l = low_thresh-6;
	      while(l < low_thresh+20 or sumo.adcDat[board_number] -> self_trig_scalar[i] > 0){ //begin fine grained scan
		sumo.adjust_thresh(l,board_number); //adjust the treshold
		usleep(1e6); //let the DAC settle in
		sumo.measure_rate(AC_read_mask); //take a noise measurement.
		
		// cout << "THRESH " << l << " noise " << sumo.adcDat[board_number]->self_trig_scalar[i] << endl;
		rate_fs << "\t" << i+1 << "\t" << l << "\t" << sumo.adcDat[board_number]->self_trig_scalar[i] << "\t" << endl; //write to file.
		l = l+1; 
		cout << " THRESH " << l-1 << " noise " << sumo.adcDat[board_number]->self_trig_scalar[i] << endl;
	      }
	      j = l;
	    }
	    
	    else j = j+5;
	  }
	}
      }
    }
    
    
    
	  
    
    rate_fs.close();//
    sumo.dump_data();
    
    
    return 0;
    
}

