#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SuMo.h"
/* associated .cpp files: */
#include "makePedandLin.cpp"

using namespace std;

SuMo::SuMo()
{
  for(int i=0; i<numFrontBoards; i++)
    DC_ACTIVE[i] = false;
}

SuMo::~SuMo()
{
  dump_data();
  usb.freeHandles();

}

int SuMo::check_active_boards(void){
  
  int num_boards_active = 0;
  for(int targetAC = 0; targetAC < 4; targetAC++){
    if(DC_ACTIVE[targetAC] == true){
      num_boards_active ++;
    }
  }   
  return num_boards_active;
}

int SuMo::check_active_boards(int NUM){
  int temp = 0;
  //set_usb_read_mode(16);
  while(check_active_boards() == 0){
      read_CC(false, false);
      temp++;
      if(temp > NUM){
	cout << "failed to find connected ADC boards" << endl;
	return 1;
      }
    }
  return 0;
}

int SuMo::read_CC(bool SHOW_CC_STATUS, bool SHOW_AC_STATUS){
  sync_usb(0);
  bool print = false; /* print buffer to terminal */
  int samples;
  unsigned short buffer[cc_buffersize];

  memset(buffer, 0x0, cc_buffersize*sizeof(unsigned short));
  
  /* set usb read mode for numFrontBoards+1 (central card only) */
  set_usb_read_mode(4);

  if(usb.createHandles() == stdUSB::SUCCEED){
    try{
      usb.readData(buffer, cc_buffersize+2, &samples); 
      if(samples < 0) return -1;

      for(int i = 0; i < cc_buffersize; i++){ 
	CC_INFO[i] = buffer[i];
	if(print) cout << i << ":" << buffer[i] << " ";
      }
      if(print) cout << "samples received: " << samples << endl;
    }
    catch(...){
      fprintf(stderr, "Please connect the board. [DEFAULT exception] \n");
      return 1;
    }
    if(SHOW_CC_STATUS){  
      cout << endl;
      cout << "AC/DC connection status: \n";
    }
    
    /* look for ACDC boards, (un)set global variable DC_ACTIVE[numFrontBoards] */
    if(buffer[2] & 0x1){ DC_ACTIVE[0] = true; 
      if(SHOW_CC_STATUS) cout <<"* DC 0 detected!! \n";}
    if(buffer[2] & 0x2){ DC_ACTIVE[1] = true;
      if(SHOW_CC_STATUS) cout <<"* DC 1 detected!! \n";}
    if(buffer[2] & 0x4){ DC_ACTIVE[2] = true; 
      if(SHOW_CC_STATUS) cout <<"* DC 2 detected!! \n";}
    if(buffer[2] & 0x8){ DC_ACTIVE[3] = true;
      if(SHOW_CC_STATUS) cout <<"* DC 3 detected!! \n";}
    
    /* cout board meta-data */
    if(SHOW_AC_STATUS){
      read_AC(0,DC_ACTIVE,false);
      for(int board=0; board<numFrontBoards; board++)
	if(DC_ACTIVE[board]){
	  cout << endl << "AC/DC #" << board << ":";
	  get_AC_info(true, board);
	}

      manage_cc_fifo(1);
    }
    
    if(SHOW_CC_STATUS) cout << "\n**********************\n";
    
    usb.freeHandles();
    return 0;
  }
  else
    return 1;
}

int SuMo::read_AC(unsigned int trig_mode, bool* mask, bool FILESAVE){
  sync_usb(0);
  bool print = false; /* switch for dumping info to terminal */
  int  numBoardsRead = 0; /* function returns number of front end cards successfully readout (int) */
  
  /* send trigger over USB from this function, if specified */
  unsigned int trig_mask = 0;
  if(!trig_mode){
    for(int boardAddress=0; boardAddress < numFrontBoards; boardAddress++)
      trig_mask = (mask[boardAddress] << boardAddress) | trig_mask; 
    if(print) cout << "trig mask " << trig_mask << endl;
    software_trigger(trig_mask);
  }

  /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for retval handling*/

  for(int boardAddress=0; boardAddress < numFrontBoards; boardAddress++){
    BOARDS_READOUT[boardAddress] = false;
    BOARDS_TIMEOUT[boardAddress] = false;
    if(DC_ACTIVE[boardAddress] == false || mask[boardAddress] == false){
      continue;
    }
    if(print) cout << "reading board: " << boardAddress << endl; 

    set_usb_read_mode(boardAddress);
    int samples;
    unsigned short buffer[ac_buffersize];
    memset(buffer, 0x0, (ac_buffersize+2)*sizeof(unsigned short));
	  
    ofstream rawDatFile;
    if(FILESAVE){
      char rawDatFilename[200];
      sprintf(rawDatFilename, "acdc-raw-data-%i.txt", boardAddress);
      rawDatFile.open(rawDatFilename, ios::trunc);
    }
    if(usb.createHandles() == stdUSB::SUCCEED){
      try{
	usb.readData(buffer, ac_buffersize+2, &samples);
	if(print) cout << "samples received: " << samples << " on board " << boardAddress << endl;

	/* packet flags */
	int pkt_header = 0;
	int pkt_footer = 0;
	int data_header = 0;
	int data_adc_footer = 0;
	int data_footer = 0;
	int usb_read_offset_flag = -1;
	  
	/* check data in packet */
	int checkpkt = 0;
	while (checkpkt<cc_buffersize){
	  acdcData[boardAddress].CC_HEADER_INFO[checkpkt] = buffer[checkpkt];
	  if(print) cout << checkpkt << ":" << buffer[checkpkt] << "  ";
	  
	  if(buffer[checkpkt]==dataPacketStart ){
	    usb_read_offset_flag = checkpkt;
	    break;
	  }
	  checkpkt++;
	}
	
	if(usb_read_offset_flag < 0){ 
	   BOARDS_TIMEOUT[boardAddress] = true;
	   if(print) cout << "timeout on board " << boardAddress << endl;
	   continue;
	}

	/* interpret data packet, save raw data to file if enabled */
	for(int i = 0; i < ac_buffersize; i++){
	  if(FILESAVE) rawDatFile << buffer[i] << "\n";     
	  if(buffer[i] == usbPacketStart )   acdcData[boardAddress].PKT_HEADER = i;
	  else if(buffer[i] == usbPacketEnd ){
	    acdcData[boardAddress].PKT_FOOTER = i;	    
	    break;
	  }
	  else if(buffer[i] == dataPacketStart ){
	    acdcData[boardAddress].DATA_HEADER[data_header] = i;
	    data_header++;
	  }
	  else if(buffer[i] == adcPacketEnd ){
	    acdcData[boardAddress].DATA_ADC_END[data_adc_footer] = i;
	    data_adc_footer++;
	  }
	  else if(buffer[i] == dataPacketEnd ){
	    acdcData[boardAddress].DATA_FOOTER[data_footer] = i;
	    data_footer++;
	  }
	}
	if(FILESAVE) rawDatFile.close();     

	CC_BIN_COUNT = (buffer[1]& 0x18) >> 3;
	CC_EVENT_NO = buffer[2];

	if(print){
	  cout << buffer[0] << "," << buffer[1] << "," << buffer[2] << ","
	       << buffer[3] << "," << buffer[4] << "," << buffer[5] << "," << endl;
	  cout << "packet header index " << acdcData[boardAddress].PKT_HEADER << endl;
	  cout << "packet data indices " << acdcData[boardAddress].DATA_HEADER[0] << ","<<acdcData[boardAddress].DATA_ADC_END[0] << ","
	       << acdcData[boardAddress].DATA_FOOTER[0] << " "	    
	       << acdcData[boardAddress].DATA_HEADER[1] << ","<<acdcData[boardAddress].DATA_ADC_END[1] << "," 
	       << acdcData[boardAddress].DATA_FOOTER[1] << " "	    
	       << acdcData[boardAddress].DATA_HEADER[2] << ","<<acdcData[boardAddress].DATA_ADC_END[2] << "," 
	       << acdcData[boardAddress].DATA_FOOTER[2] << " "	    
	       << acdcData[boardAddress].DATA_HEADER[3] << ","<<acdcData[boardAddress].DATA_ADC_END[3] << "," 
	       << acdcData[boardAddress].DATA_FOOTER[3] << " "
	       << acdcData[boardAddress].DATA_HEADER[4] << ","<<acdcData[boardAddress].DATA_ADC_END[4] << "," 
	       << acdcData[boardAddress].DATA_FOOTER[4] << endl;	  
	  cout << "packet footer index " << acdcData[boardAddress].PKT_FOOTER << endl;	    
	}
	/* real data starts here: */
	usb_read_offset_flag = usb_read_offset_flag + 2;
	/* form usable data from packets */
	for(int i = 0; i < numChipsOnBoard; i++){
	  for(int j = 0; j < psec_buffersize; j++){
	    acdcData[boardAddress].AC_RAW_DATA[i][j] = buffer[usb_read_offset_flag +
	    						      (psec_buffersize+infoBuffersize)*i+j];	
	  }
	  for(int k = 0; k < infoBuffersize; k++){
	    acdcData[boardAddress].AC_INFO[i][k] = buffer[usb_read_offset_flag +
							  (i+1)*psec_buffersize
							  + (i*infoBuffersize) + k];
	  }
	}  
	for(int i=0; i<AC_CHANNELS; i++){
	  acdcData[boardAddress].SELF_TRIG_SCALER[i] = buffer[acdcData[boardAddress].DATA_FOOTER[numChipsOnBoard-1]+2+i];
	  //if(print) cout << acdcData[boardAddress].SELF_TRIG_SCALER[i] << endl;
	}
	    
	BOARDS_READOUT[boardAddress] = true;
	numBoardsRead++;
      }
      
      catch(...){
	fprintf(stderr, "Please connect the board. [DEFAULT exception]\n");
	return 1;
      }
    }
  }    
return numBoardsRead;
    
}

int SuMo::get_AC_info(bool PRINT, int frontEnd){
  int aa = frontEnd;
  unsigned short AC_INFO[numChipsOnBoard][infoBuffersize];
  for(int i=0; i<numChipsOnBoard; i++){
    for(int j=0; j<infoBuffersize; j++){
      AC_INFO[i][j] = acdcData[aa].AC_INFO[i][j];
    }
  }
  unsigned short ref_volt_mv  = 1200;
  unsigned short num_bits     = 4096;
  for (int i = 0; i < 5; i++){
    acdcData[aa].RO_CNT[i] =          (float) AC_INFO[i][4] * 10 * pow(2,11)/ (pow(10,6));
    acdcData[aa].RO_TARGET_CNT[i] =   (float) AC_INFO[i][5] * 10 * pow(2,11)/(pow(10,6));
    acdcData[aa].VBIAS[i] =           (float) AC_INFO[i][6] * ref_volt_mv/num_bits;
    acdcData[aa].TRIGGER_THRESHOLD[i]=(float) AC_INFO[i][7] * ref_volt_mv/num_bits;
    acdcData[aa].RO_DAC_VALUE[i] =    (float) AC_INFO[i][8] * ref_volt_mv/num_bits;
  }
   
  acdcData[aa].CC_BIN_COUNT =         (acdcData[aa].CC_HEADER_INFO[1] & 0x18) >> 3;
  acdcData[aa].CC_EVENT_COUNT =       acdcData[aa].CC_HEADER_INFO[3] | acdcData[aa].CC_HEADER_INFO[2] << 16;
  acdcData[aa].CC_TIMESTAMP_LO =      acdcData[aa].CC_HEADER_INFO[4];
  acdcData[aa].CC_TIMESTAMP_MID =     acdcData[aa].CC_HEADER_INFO[5];
  acdcData[aa].CC_TIMESTAMP_HI =      acdcData[aa].CC_HEADER_INFO[6]; 

  acdcData[aa].BIN_COUNT_RISE =       AC_INFO[0][9] & 0x0F;
  acdcData[aa].BIN_COUNT_FALL =       (AC_INFO[0][9] & 0xF0) >> 4;
  
  acdcData[aa].SELF_TRIG_SETTINGS =   AC_INFO[1][9];
  acdcData[aa].TRIG_EN =              acdcData[aa].SELF_TRIG_SETTINGS & 0x1;
  acdcData[aa].TRIG_WAIT_FOR_SYS =    acdcData[aa].SELF_TRIG_SETTINGS & 0x2;
  acdcData[aa].TRIG_RATE_ONLY =       acdcData[aa].SELF_TRIG_SETTINGS & 0x4;
  acdcData[aa].TRIG_SIGN =            acdcData[aa].SELF_TRIG_SETTINGS & 0x8;
  
  //acdcData[aa].EVENT_COUNT =          AC_INFO[2][9]  | AC_INFO[2][9]  << 16;
  acdcData[aa].REG_SELF_TRIG[0] =     AC_INFO[0][10] | AC_INFO[1][10] << 16;
  acdcData[aa].REG_SELF_TRIG[1] =     AC_INFO[2][10] | AC_INFO[3][10] << 16;
  acdcData[aa].REG_SELF_TRIG[2] =     AC_INFO[4][10] | AC_INFO[0][11] << 16;
  acdcData[aa].REG_SELF_TRIG[3] =     AC_INFO[1][11] | AC_INFO[2][11] << 16;
  acdcData[aa].SELF_TRIG_MASK =       AC_INFO[3][11] | AC_INFO[4][11] << 16;
  acdcData[aa].LAST_AC_INSTRUCT =     AC_INFO[0][12] | AC_INFO[1][12] << 16;
  acdcData[aa].LAST_LAST_AC_INSTRUCT= AC_INFO[2][12] | AC_INFO[3][12] << 16;
  acdcData[aa].EVENT_COUNT =          AC_INFO[3][13] | AC_INFO[4][13] << 16;
  acdcData[aa].TIMESTAMP_HI=          AC_INFO[2][13];
  acdcData[aa].TIMESTAMP_MID=         AC_INFO[1][13];
  acdcData[aa].TIMESTAMP_LO=          AC_INFO[0][13];

  if(PRINT){
    cout << std::fixed;
    cout << std::setprecision(2);
    cout << std::dec << endl;
    cout << "central card header info: " << endl;
    cout << "bin:" << acdcData[aa].CC_BIN_COUNT
	 << " evt.count:" << acdcData[aa].CC_EVENT_COUNT
	 << " timestamp: " <<  acdcData[aa].CC_TIMESTAMP_HI <<":"<<acdcData[aa].CC_TIMESTAMP_MID 
	 <<":"<<acdcData[aa].CC_TIMESTAMP_LO<< endl;
    cout << "--------" << endl;
    cout << "event count: " << std::dec << acdcData[aa].EVENT_COUNT; 
    cout << " board time: " << acdcData[aa].TIMESTAMP_HI<<":"<<acdcData[aa].TIMESTAMP_MID<<":"<<acdcData[aa].TIMESTAMP_LO;
    //cout << " LAST TRIG TIMESTAMP: " << acdcData[aa].TIMESTAMP;
    cout << endl;
    cout << "last instructions: 0x" <<  std::hex << acdcData[aa].LAST_AC_INSTRUCT 
	 << ", 0x" << acdcData[aa].LAST_LAST_AC_INSTRUCT << endl;
    cout << "registered self-trig bits: 0x" << hex << acdcData[aa].REG_SELF_TRIG[0] 
	 <<", 0x"                           << hex << acdcData[aa].REG_SELF_TRIG[1]
	 <<", 0x"                           << hex << acdcData[aa].REG_SELF_TRIG[2]
         <<", 0x"                           << hex << acdcData[aa].REG_SELF_TRIG[3] 
	 << endl;
    cout << "self-trig mask: 0x"     << hex << acdcData[aa].SELF_TRIG_MASK << endl;
    cout << "self-trig settings: 0x" << hex << acdcData[aa].SELF_TRIG_SETTINGS << endl;  
    cout << "trig_sign: "            << acdcData[aa].TRIG_SIGN 
	 << ", wait_for_sys: "       << acdcData[aa].TRIG_WAIT_FOR_SYS
	 << ", rate_only: "          << acdcData[aa].TRIG_RATE_ONLY
	 << ", EN: "                 << acdcData[aa].TRIG_EN 
	 << endl;
    cout << "bin count rise edge: "  << acdcData[aa].BIN_COUNT_RISE 
	 << ", bin count fall edge: "<< acdcData[aa].BIN_COUNT_FALL
         << endl;

    for (int i = 0; i < 5; i++){
      cout << "PSEC:" << i;
      cout << "|ADC clock/trgt:"     << acdcData[aa].RO_CNT[i];
      cout << "/"                    << acdcData[aa].RO_TARGET_CNT[i] << "MHz";
      cout << ",bias:"               << acdcData[aa].RO_DAC_VALUE[i] <<"mV";
      cout << "|Ped:"                << acdcData[aa].VBIAS[i] << "mV";
      cout << "|Trig:"               << acdcData[aa].TRIGGER_THRESHOLD[i] << "mV";
      cout << endl;
    }
  } 
  return 0;
}

//unwrap locally on AC/DC
/*
int SuMo::unwrap(int ASIC){
  int last_sampling_bin = LAST_SAMPLING_BIN[ASIC];
  unsigned int BIN_40[4];

  for(int k=0; k<4; k++){
    BIN_40[k] = (int)256/4*k;
  }

  return BIN_40[last_sampling_bin];
}
*/

int SuMo::unwrap(int ASIC){
  int last_sampling_bin = CC_BIN_COUNT;
  //int last_sampling_bin = 0;
  unsigned int BIN_40[4];

  //note BIN_40 assuming 160MHz triggering clock

  for(int k=0; k<4; k++){
    BIN_40[k] = (int)256/4*k;
  }
  return BIN_40[last_sampling_bin];
}

void SuMo::unwrap_baseline(int *baseline, int ASIC){
  int start_baseline[256];
  for(int i = 0; i < 256; i++){
    start_baseline[i] = i;
  }
  
  int wraparound = unwrap(ASIC);

  if((wraparound+WRAP_CONSTANT) < 257)
    wraparound = wraparound+WRAP_CONSTANT;
  else
    wraparound = wraparound+WRAP_CONSTANT-256;

  //printf("%d", wraparound);
  for(int i = 0; i < 256; i++){
    baseline[i] = start_baseline[ (wraparound + i) % 256];
  }
}
