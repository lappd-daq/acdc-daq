#include "SuMo.h"
#include <algorithm>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

using namespace std;

const unsigned int LINEARITY_SCAN_START    = 10; /* ADC counts */
const unsigned int LINEARITY_SCAN_STEPSIZE = 80; /* ADC counts */
const unsigned int DAC_RANGE               = 4096; /* ADC counts */

int SuMo::generate_ped(bool ENABLE_FILESAVE){
  sync_usb(0);
  unsigned int middle = int(num_ped_reads/2);
  unsigned int psec_cnt;
  float temp;

  bool verbose = false;

  bool all[numFrontBoards];
  for(int i=0;i<numFrontBoards; i++) all[i] = true;
  
  if(mode==USB2x) set_usb_read_mode_slaveDevice(16);
  set_usb_read_mode(16);
  dump_data();

  int bin = 0;

  for(int k=0; k<num_ped_reads; k++){   
    /*set read mode to NULL */
    set_usb_read_mode(16);
    if(mode==USB2x) set_usb_read_mode_slaveDevice(16);
    /*reset last event on firmware */
    manage_cc_fifo(1);
    if(mode==USB2x) manage_cc_fifo_slaveDevice(1);

    // send software trigger, integrated over all trigger bins
    bin = k % 4;
    /* read data using trigger over software */
    //read_AC(0, DC_ACTIVE, false);
    read_AC(0, DC_ACTIVE, false, false, true, bin);
        
    /* make pedestal files for each board */
    for(int targetAC=0; targetAC < numFrontBoards; targetAC++){    
      if(!BOARDS_READOUT[targetAC]) continue;

      calData[targetAC].PED_SUCCESS = true;
      psec_cnt = 0;
      for(int i=0; i<AC_CHANNELS; i++){
	if(i>0 && i % 6 == 0) psec_cnt ++;
	/* make raw arrays from incoming data */
	for(int j=0; j<psecSampleCells; j++){
	  calData[targetAC].raw_ped_data_array[i][j][k] = adcDat[targetAC]->AC_RAW_DATA[psec_cnt][i%6*256+j];
	}
      }
    } /* end front-end board loop */
  } /* end num readouts loop */
  
  /* calculate pedestal value median and RMS  */
  for(int board=0; board < numFrontBoards; board++){   
    if(calData[board].PED_SUCCESS){
      for(int i = 0; i<AC_CHANNELS; i++){
	
	if(verbose) cout << "ch." << i << ":";
	
	for(int j = 0; j<psecSampleCells; j++){
	  /* calc RMS */
	  qsort(calData[board].raw_ped_data_array[i][j], num_ped_reads, sizeof(unsigned short), compare);
	  calData[board].PED_DATA[i][j] = (unsigned int)calData[board].raw_ped_data_array[i][j][middle];
	  temp = 0;
	  for(int k = 0; k<num_ped_reads; k++) { 
	    temp += pow((calData[board].raw_ped_data_array[i][j][k]-
			 (unsigned short)calData[board].PED_DATA[i][j]),2);
	  }
	  calData[board].PED_RMS[i][j] = sqrt(temp/num_ped_reads);
	  if(verbose && j==0) cout << "ped[0]=" << calData[board].PED_DATA[i][j] << " | ";
	}
	if(verbose) 
	  if( (i+1)%6 == 0) cout << endl;
      }   
      if(verbose) cout << endl;
      /*done with calculations*/
      /*save to file, if specified*/
      if(ENABLE_FILESAVE){
	char ped_filename[200], ped_rms_filename[200];
	mkdir("calibrations", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sprintf(ped_filename, "calibrations/PED_DATA_%d.txt", board);
	sprintf(ped_rms_filename, "calibrations/PED_DATA_RMS_%d.txt", board);

	ofstream fped, frms;
	fped.open(ped_filename, ios::trunc);
	frms.open(ped_rms_filename, ios::trunc);
      
	for(int i=0; i < psecSampleCells; i++){
	  fped << i << "\t";
	  frms << i << "\t";
	  for(int channel=0; channel < AC_CHANNELS; channel++){
	    fped << std::dec << calData[board].PED_DATA[channel][i] << "\t";
	    frms << std::dec << calData[board].PED_RMS[channel][i] << "\t";
	  }
	  fped << endl;
	  frms << endl;
	}
	fped << endl << endl;
	frms << endl << endl;
      
	fped.close();
	frms.close();
	cout << "Pedestal values saved to file: " << ped_filename << endl << "___" << endl;;
      }
    }
  }
  return 0;
    
}

int SuMo::put_lut_on_heap(bool* range){

  LUT_CELL = new float**[numFrontBoards];
  LUT = new float**[numFrontBoards];
  
  for(int i=0; range[i]==true; i++){ 
    LUT_CELL[i] = new float*[AC_CHANNELS*psecSampleCells];
    LUT[i] = new float*[AC_CHANNELS];

    for(int j=0; j<AC_CHANNELS; j++){
      for(int k=0; k<psecSampleCells; k++) 
	LUT_CELL[i][j*psecSampleCells+k] = new float[4096];
      LUT[i][j] = new float[4096];
      
    }
  }
  return 0;
}

void SuMo::free_lut_from_heap(bool* range){
 for(int i=0; range[i]==true; i++){ 
   for(int j=0; j<AC_CHANNELS; j++){
     for(int k=0; k<psecSampleCells; k++)
       delete [] LUT_CELL[i][j*psecSampleCells+k];
     delete [] LUT[i][j];
   }
   delete [] LUT_CELL[i];
   delete [] LUT[i];
  }
 delete [] LUT;
 delete [] LUT_CELL;

}

int SuMo::make_count_to_voltage(void){
  /* move these to arguments, eventually */
  bool COPY = false;
  bool boardsToMakeLUT[numFrontBoards];
  for(int i=0; i<numFrontBoards; i++) boardsToMakeLUT[i] = true;

  /* try to put count-to-voltage arrays on the heap */
  put_lut_on_heap(boardsToMakeLUT);

  char LUT_filename[100],  LUT_all_filename[100], raw_scan_filename[100], raw_scan_all_filename[100];;
  ofstream flut[numFrontBoards], flut_all[numFrontBoards], ftemp[numFrontBoards], ftemp_all[numFrontBoards];
  unsigned int dac_level, temp[AC_CHANNELS], temp_all[AC_CHANNELS][psecSampleCells];

  /* initiate, open filestreams, save old LUT data, if specified */
  for(int targetAC = 0; targetAC < numFrontBoards; targetAC++){
    if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC])
      cout << "making count-to-conversion LUT for board " << targetAC << endl;
    else continue;
    
    sprintf(LUT_filename, "calibrations/LUT_%d.txt", targetAC);
    sprintf(LUT_all_filename, "calibrations/LUT_CELL_%d.txt", targetAC);
    sprintf(raw_scan_filename, "calibrations/raw_lin_scan_%d.txt", targetAC);
    sprintf(raw_scan_all_filename, "calibrations/raw_lin_scan_cell_%d.txt", targetAC);

    if(COPY){
      /* save to back-up file to-do */
      /*....*/
    }
    flut[targetAC].open(LUT_filename, ios::trunc);
    flut_all[targetAC].open(LUT_all_filename, ios::trunc);
    ftemp[targetAC].open(raw_scan_filename, ios::trunc);
    ftemp_all[targetAC].open(raw_scan_all_filename, ios::trunc);

    for( int i = 0; i< AC_CHANNELS; i++){
      for( int j = 0; j<DAC_RANGE; j++) LUT[targetAC][i][j] = -1;
    }
    for( int i = 0; i< AC_CHANNELS; i++){
      for( int k = 0; k<psecSampleCells; k++){
	for( int j = 0; j<DAC_RANGE; j++)
	  LUT_CELL[targetAC][i*psecSampleCells+k][j] = -1;
      }
    }
  } /* end front-end board loop */

  int level_number = 0;
  /* looping thru DAC values, defined at top of file */
  for( int i = LINEARITY_SCAN_START; i < DAC_RANGE; i+=LINEARITY_SCAN_STEPSIZE){
    dac_level = i;
    float voltage = i * 1.200/4096.0;
    
    /* set ped. redunancy probably shouldn't be necessary */
    /* uncomment inorder to make sure ped value on board is equal before recording  */
    //for(int targetAC = 0; targetAC < numFrontBoards; targetAC++){
    //  if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC]){      
    //	bool OK_GO=false;
    //	while(!OK_GO){
    //	  set_usb_read_mode(16); 
    //	  set_pedestal_value(i);
    //	  usleep(1000);
    //	  read_AC(0,DC_ACTIVE, false);
    //	  get_AC_info(false, targetAC);
    //	  //cout << acdcData[targetAC].VBIAS[0]<< ":" << i << endl;
    //	  if(acdcData[targetAC].VBIAS[0]==i && acdcData[targetAC].VBIAS[3]==i){
    //	    //cout << targetAC <<":"<<acdcData[targetAC].VBIAS[0]<<":"<<i<<endl;
    //	    read_AC(0,DC_ACTIVE, false);
    //	    get_AC_info(false, targetAC);
    //	    if(acdcData[targetAC].VBIAS[0]==i && acdcData[targetAC].VBIAS[3]==i) OK_GO=true;
    //	  }
    //	}
    //}
    //}
  
    set_usb_read_mode(16); 
    if(mode==USB2x) set_usb_read_mode_slaveDevice(16); 
    set_pedestal_value(i, 15, 0); //once
    if(mode==USB2x) set_pedestal_value(i, 15, 1);
    set_pedestal_value(i, 15, 0); //twice
    if(mode==USB2x) set_pedestal_value(i, 15, 1);
    set_pedestal_value(i, 15, 0); //3 times for good measure
    if(mode==USB2x) set_pedestal_value(i, 15, 1);

    cout << "Taking scan point " << level_number
	 << " of " << floor ((DAC_RANGE-LINEARITY_SCAN_START)/LINEARITY_SCAN_STEPSIZE)
	 << " @ " << i << " DAC counts & " << voltage << " Volts    \r";
    cout.flush();

    sleep(5); /* allow DAC voltage to settle */
    /* generate cell pedestal */    
    generate_ped(false);
    //cout << endl;        //uncomment to print ACDC status at each scan point
    //read_CC(true, true); //uncomment to print ACDC status at each scan point
			     
    for(int targetAC = 0;  targetAC <numFrontBoards; targetAC++){
      if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC]){

	for( int j = 0; j< AC_CHANNELS; j++) temp[j] = 0;
	
	ftemp[targetAC] << i << "\t";
	ftemp_all[targetAC] << i << "\t";

	for(int m=0;m<AC_CHANNELS; m++){
	  for(int j=0;j<psecSampleCells; j++){
	    temp[m] += (unsigned int) calData[targetAC].PED_DATA[m][j];
	    temp_all[m][j] = (unsigned int) calData[targetAC].PED_DATA[m][j];
	    ftemp_all[targetAC] << temp_all[m][j] << "\t";
	    /* assign value to cell-level LUT */	   
	    if (temp_all[m][j] >= 0 && temp_all[m][j] < DAC_RANGE){
	      LUT_CELL[targetAC][m*psecSampleCells+j][temp_all[m][j]]=voltage;
	      //cout << temp_all[m][j] << ":"<<LUT_CELL[targetAC][m][temp_all[m][j]][j] << ",";
	    }
	  }
	  //cout << endl;
	  temp[m] /= psecSampleCells;
	  ftemp[targetAC]<< temp[m] <<  "\t"; 
	  /* assign value to channel-level LUT */
	  if (temp[m] >= 0 && temp[m] < DAC_RANGE) LUT[targetAC][m][temp[m]] = voltage;  
	  
	}	
	ftemp[targetAC] << endl;
	ftemp_all[targetAC] << endl;

      }
    }
    level_number++;
  }
  cout << endl;
  for(int targetAC = 0; targetAC <numFrontBoards; targetAC++){   
    if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC]){
      ftemp[targetAC].close();
      ftemp_all[targetAC].close();
    }
  }
  /* this ends physical pedestal scan on board */
  /* now interpolate and save linearity calibration files: */
  cout << "Now, computing linear interpolations of raw scan data..." << endl;
    
  for(int targetAC = 0; targetAC <numFrontBoards; targetAC++){   
    if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC]){
      /* do linear interpolations for channel-level LUT*/
      int first, second, j;
      for(int i=0; i < AC_CHANNELS; i++) {  /* for each channel */
	// find first value
	for(j=0; LUT[targetAC][i][j] < 0; j++);  
	// don't test equality with floating point #s        
	first = j;  // first is now the first LUT value
	while (j < DAC_RANGE) {
	  for(j++; LUT[targetAC][i][j] < 0 && j < DAC_RANGE; j++);
          
	  if (j >= DAC_RANGE) continue;
          
	  second = j;
          
	  for(int k=first+1; k < second; k++) {
	    LUT[targetAC][i][k] = LUT[targetAC][i][first] + 
	      (float)(k-first)/(second-first) * 
	      (LUT[targetAC][i][second] - LUT[targetAC][i][first] );
	  }
	  
	  first = second;
	}
      }
    }
  }
  cout << "Now, computing linear interpolations of raw cell scan data..." << endl;

  int index = 0;
  for(int targetAC = 0; targetAC <numFrontBoards; targetAC++){   
    if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC]){    
      int first, second, j;
      /* do linear interpolations for cell-level LUT */
      for(int i=0; i < AC_CHANNELS; i++) {  /* for each channel */
	for(int cell=0; cell<psecSampleCells; cell++){ /* for each cell */
	  index = i*psecSampleCells+cell;
	  // find first value that is registered
	  for(j=0; LUT_CELL[targetAC][index][j] < 0; j++);  

	  // don't test equality with floating point #s        
	  first = j;  // first is now the first LUT value
	  while (j < DAC_RANGE) {

	    for(j++; LUT_CELL[targetAC][index][j] < 0 && j < DAC_RANGE; j++);
	    
	    if (j >= DAC_RANGE) continue;
	    
	    second = j;
	    
	    for(int k=first+1; k < second; k++) {
	      LUT_CELL[targetAC][index][k] = LUT_CELL[targetAC][index][first] + 
		(float)(k-first)/(second-first) * 
		(LUT_CELL[targetAC][index][second] - LUT_CELL[targetAC][index][first] );
	    }
	    
	    first = second;
	  }
	}
      }
    }
  }
  cout << "Now, saving data to file..." << endl;

  for(int targetAC = 0; targetAC <numFrontBoards; targetAC++){   
    if(DC_ACTIVE[targetAC] && boardsToMakeLUT[targetAC]){ 
      for(int i=0; i < DAC_RANGE; i++){
	flut[targetAC] << i << "\t";
	flut_all[targetAC] << i << "\t";
	
	for(int channel=0; channel < AC_CHANNELS; channel++){
	  flut[targetAC] << LUT[targetAC][channel][i] << "\t";
	  
	  for(int cell=0; cell < psecSampleCells; cell++){ 
	    flut_all[targetAC] << LUT_CELL[targetAC][channel*psecSampleCells+cell][i] << "\t"; 
	  }
	}
	
	flut[targetAC] << endl;
	flut_all[targetAC] << endl;
	
      }
      flut[targetAC] << endl;
      flut_all[targetAC] << endl;
      
      flut[targetAC].close();
      flut_all[targetAC].close();
    }
  }

  free_lut_from_heap(boardsToMakeLUT);
  cout << "Done. Count to voltage LUTs saved to file" << endl;;
  cout << "**********" << endl;;
  return 0;
}
