#include "SuMo.h"

using namespace std;

int SuMo::read_AC(unsigned int trig_mode, bool* mask, bool FILESAVE, 
		  bool sync, bool set_bin, unsigned int bin){

  bool print = false;     /* verbose switch for dumping info to terminal */
  int  numBoardsRead = 0; /* function returns number of front end cards successfully readout (int) */
  
  /* send trigger over USB from this function, if specified */
  unsigned int trig_mask = 0;
  unsigned int trig_mask_slave = 0;

  if(!trig_mode){
    //loop over master board
    for(int boardAddress=0; boardAddress < boardsPerCC; boardAddress++)
      trig_mask = (mask[boardAddress] << boardAddress) | trig_mask; 
    if(print) cout << "trig mask " << trig_mask << endl;
  
    //temporarily loop over slave board, eventually connected in firmware
    for(int boardAddress=boardsPerCC; boardAddress < numFrontBoards; boardAddress++)
      trig_mask_slave = (mask[boardAddress] << boardAddress-4) | trig_mask_slave; 
    if(print) cout << "trig mask on slave " << trig_mask_slave << endl;

    if(sync){
      prep_sync();
      software_trigger(trig_mask, set_bin, bin);
      software_trigger_slaveDevice(trig_mask_slave, set_bin, bin);
      make_sync();
    }
    else{
      software_trigger(trig_mask, set_bin, bin);
      software_trigger_slaveDevice(trig_mask_slave, set_bin, bin);
    }
  }

  /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for retval handling*/

  for(int boardAddress=0; boardAddress < numFrontBoards; boardAddress++){
    BOARDS_READOUT[boardAddress] = false;
    BOARDS_TIMEOUT[boardAddress] = false;
    if(DC_ACTIVE[boardAddress] == false || mask[boardAddress] == false){
      continue;
    }
    int device = 0;                                 //default 'master' device
    if(boardAddress >= boardsPerCC && mode==USB2x)  //specify slave device if conditions satisfied
      device = 1;                                      

    if(print) cout << "reading board: " << boardAddress << endl; 

    if(device == 1) set_usb_read_mode_slaveDevice(boardAddress+1-boardsPerCC);
    else            set_usb_read_mode(boardAddress+1);
    
    int samples;
    unsigned short buffer[ac_buffersize];
    memset(buffer, 0x0, (ac_buffersize+2)*sizeof(unsigned short));
	  
    ofstream rawDatFile;
    if(FILESAVE){
      char rawDatFilename[200];
      sprintf(rawDatFilename, "acdc-raw-data-%i.txt", boardAddress);
      rawDatFile.open(rawDatFilename, ios::trunc);
    }

    /* try to get packet from addressed AC/DC card */
    try{
      if(device == 1) usb2.readData(buffer, ac_buffersize+2, &samples);
      else            usb.readData(buffer, ac_buffersize+2, &samples);
      
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
	adcDat[boardAddress]->CC_HEADER_INFO[checkpkt] = buffer[checkpkt];
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
	if(buffer[i] == usbPacketStart )   adcDat[boardAddress]->PKT_HEADER = i;
	else if(buffer[i] == usbPacketEnd ){
	  adcDat[boardAddress]->PKT_FOOTER = i;	    
	  break;
	}
	else if(buffer[i] == dataPacketStart ){
	  adcDat[boardAddress]->DATA_HEADER[data_header] = i;
	  data_header++;
	}
	else if(buffer[i] == adcPacketEnd ){
	  adcDat[boardAddress]->DATA_ADC_END[data_adc_footer] = i;
	  data_adc_footer++;
	}
	else if(buffer[i] == dataPacketEnd ){
	  adcDat[boardAddress]->DATA_FOOTER[data_footer] = i;
	  data_footer++;
	}
      }
      if(FILESAVE) rawDatFile.close();     
      
      CC_BIN_COUNT = (buffer[1]& 0x18) >> 3;
      CC_EVENT_NO = buffer[2];
      
      if(print){
	cout << buffer[0] << "," << buffer[1] << "," << buffer[2] << ","
	     << buffer[3] << "," << buffer[4] << "," << buffer[5] << "," << endl;
	cout << "packet header index " << adcDat[boardAddress]->PKT_HEADER << endl;
	cout << "packet data indices " << adcDat[boardAddress]->DATA_HEADER[0] << ","<<adcDat[boardAddress]->DATA_ADC_END[0] << ","
	     << adcDat[boardAddress]->DATA_FOOTER[0] << " "	    
	     << adcDat[boardAddress]->DATA_HEADER[1] << ","<<adcDat[boardAddress]->DATA_ADC_END[1] << "," 
	     << adcDat[boardAddress]->DATA_FOOTER[1] << " "	    
	     << adcDat[boardAddress]->DATA_HEADER[2] << ","<<adcDat[boardAddress]->DATA_ADC_END[2] << "," 
	     << adcDat[boardAddress]->DATA_FOOTER[2] << " "	    
	     << adcDat[boardAddress]->DATA_HEADER[3] << ","<<adcDat[boardAddress]->DATA_ADC_END[3] << "," 
	     << adcDat[boardAddress]->DATA_FOOTER[3] << " "
	     << adcDat[boardAddress]->DATA_HEADER[4] << ","<<adcDat[boardAddress]->DATA_ADC_END[4] << "," 
	     << adcDat[boardAddress]->DATA_FOOTER[4] << endl;	  
	cout << "packet footer index " << adcDat[boardAddress]->PKT_FOOTER << endl;	    
      }
      /* real data starts here: */
      usb_read_offset_flag = usb_read_offset_flag + 2;
      /* form usable data from packets */
      for(int i = 0; i < numChipsOnBoard; i++){
	for(int j = 0; j < psec_buffersize; j++){
	  adcDat[boardAddress]->AC_RAW_DATA[i][j] = buffer[usb_read_offset_flag +
							   (psec_buffersize+infoBuffersize)*i+j];	
	}
	for(int k = 0; k < infoBuffersize; k++){
	  adcDat[boardAddress]->AC_INFO[i][k] = buffer[usb_read_offset_flag +
						       (i+1)*psec_buffersize
						       + (i*infoBuffersize) + k];
	}
      }  
      for(int i=0; i<AC_CHANNELS; i++){
	adcDat[boardAddress]->self_trig_scalar[i] = buffer[adcDat[boardAddress]->DATA_FOOTER[numChipsOnBoard-1]+2+i];
	if(print) cout << adcDat[boardAddress]->self_trig_scalar[i] << endl;
      }
      
      BOARDS_READOUT[boardAddress] = true;
      numBoardsRead++;
    }
      
    catch(...){
      fprintf(stderr, "Please connect the board. [DEFAULT exception]\n");
      return 1;
    }
  
  }    
  return numBoardsRead;
    
}
