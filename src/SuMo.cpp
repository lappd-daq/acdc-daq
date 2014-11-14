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
  set_usb_read_mode(16);
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
  set_usb_read_mode(4);
 
  bool tt = SHOW_AC_STATUS;
  int samples;
  unsigned short buffer[cc_buffersize];

  memset(buffer, 0x0, cc_buffersize*sizeof(unsigned short));
  
  if(usb.createHandles() == stdUSB::SUCCEED){
    try{
      usb.readData(buffer, cc_buffersize+2, &samples);
      //printf("received: %d samples\n", samples);
      
      for(int i = 0; i < cc_buffersize; i++){
	CC_INFO[i] = buffer[i];
	if(SHOW_CC_STATUS)
	  printf("%x, ", CC_INFO[i]);
      }
      if(SHOW_CC_STATUS)
	printf("\n");
    }
    catch(...){
      fprintf(stderr, "Please connect the board. [DEFAULT exception] \n");
      return 1;
    }
    if(SHOW_CC_STATUS){  
      printf(" CC BIN COUNT: %d\n", CC_BIN_COUNT);
      printf( "Central Card USB connection status: ");
    
      if(buffer[1] == 0xDEAD && buffer[13] == 0xBEEF)
	printf(" ONLINE\n");
      else
	printf(" OFFLINE. check USB connection\n");
      
      printf("ACDC connection status: \n");
    }
    /* look for ACDC 1 */
    if(buffer[2] & 0x1){
      DC_ACTIVE[0] = true;
      if(SHOW_CC_STATUS) printf("* DC 1 detected!! \n");
      if(tt){
	read_AC(false,0,0);
	get_AC_info(true, 0);
	manage_cc_fifo(1);
      }      
    }
    else{
      if(SHOW_CC_STATUS) printf("* DC 1 not connected \n");
      DC_ACTIVE[0] = false;
    }
    /* look for ACDC 2 */
    if(buffer[2] & 0x2){
      if(SHOW_CC_STATUS) printf("* DC 2 detected!! \n");
      DC_ACTIVE[1] = true;
      if(tt){
	read_AC(false,0,1);
	get_AC_info(true, 1);
	manage_cc_fifo(1);
      }    
    }
    else{
      if(SHOW_CC_STATUS) printf("* DC 2 not connected \n");      
      DC_ACTIVE[1] = false;
    }
    /* look for ACDC 3 */   
    if(buffer[2] & 0x4){
      if(SHOW_CC_STATUS) printf("* DC 3 detected!! \n");
      DC_ACTIVE[2] = true;
      if(tt){
	read_AC(false,0,2);
	get_AC_info(true, 2);
	manage_cc_fifo(1);
      }    
    }
    else{
      if(SHOW_CC_STATUS) printf("* DC 3 not connected \n");  
      DC_ACTIVE[2] = false;
    }
    /* look for ACDC 4 */   
    if(buffer[2] & 0x8){
      if(SHOW_CC_STATUS) printf("* DC 4 detected!! \n");
      DC_ACTIVE[3] = true;
      if(tt){
	read_AC(false,0,3);
	get_AC_info(true, 3);
	manage_cc_fifo(1);
      }    
    }
    else{
      if(SHOW_CC_STATUS) printf("* DC 4 not connected \n"); 
      DC_ACTIVE[3] = false;
    }
    if(SHOW_CC_STATUS) printf("*******\n");
    
    //manage_cc_fifo(1);
    usb.freeHandles();

  //delete[] buffer;
    return 0;
  }
  else
    return 1;
}

int SuMo::read_AC(bool ENABLE_FILESAVE, unsigned int trig_mode, int AC_adr){
  sync_usb(0);

  //if(DC_ACTIVE[AC_adr] == false){
  //  printf("no AC detected at specified address\n");
  //  return 2;
  //}
  set_usb_read_mode(AC_adr);
  if(!trig_mode) software_trigger(1 << AC_adr);
  int samples;
  int usb_second_offset_flag;
  unsigned short buffer[ac_buffersize];
  memset(buffer, 0x0, (ac_buffersize+2)*sizeof(unsigned short));
  
  if(usb.createHandles() == stdUSB::SUCCEED){
    try{
      usb.readData(buffer, ac_buffersize+2, &samples);
      //manage_cc_fifo(1);
      //if(ENABLE_FILESAVE) printf("received: %d samples\n", samples);
      
      if(ENABLE_FILESAVE){
	ofstream raw_data_file;
	char raw_read_filename[200];
	sprintf(raw_read_filename, "AC_raw_read_%i.txt", AC_adr);
	raw_data_file.open(raw_read_filename, ios::trunc);

	for(int i = 0; i <ac_buffersize; i++){
	  raw_data_file << buffer[i] << "\n";
	  //if(buffer[i] ==0 && i<1000)	  
	    //printf("%i:%x, ", i,buffer[i]);
	}
	//printf("\n");
	raw_data_file.close();     
      }

      int usb_read_offset_flag = -1;
      for(int i = 0; i < 100; i++){
	if(buffer[i] == 0xF005 ){
	  //printf("AC=%d samples=%d i=%d %d %d %d %X, %X, %d %d %d\n", 
	  //	 AC_adr,samples,i, buffer[i-4],buffer[i-3],buffer[i-2],buffer[i-1],buffer[i],
	  //	 buffer[i+1], buffer[i+2], buffer[i+3]);
	  usb_read_offset_flag = i;
	  if(usb_read_offset_flag==6){
	    CC_BIN_COUNT = (buffer[i-5]& 0x18) >> 3;
	    CC_EVENT_NO = buffer[i-4];
	  }
	  else if(usb_read_offset_flag==5){
	    CC_BIN_COUNT = (buffer[i-4]& 0x18) >> 3;
	    CC_EVENT_NO = buffer[i-3];
	  }
	  //printf("usb offset = %d\n", usb_read_offset_flag);
	  break;
	}
      }
      for(int i = 1400; i < 1600; i++){
	if(buffer[i] == 0xBA11 ){
	  usb_second_offset_flag = i;
	  //printf("usb 2 offset = %d\n", usb_second_offset_flag);
	  break;
	}
      }

      if(usb_read_offset_flag < 0){
	//printf("USB read header word not found\n");
	return -1;
      }
      /* 'real' data starts here: */
      usb_read_offset_flag = usb_read_offset_flag + 2; 
      
      for(int i = 0; i < numChipsOnBoard; i++){
	for(int j = 0; j < psec_buffersize; j++){
	  acdcData[AC_adr].AC_RAW_DATA[i][j] = buffer[usb_read_offset_flag +
			      (psec_buffersize+infoBuffersize)*i+j];	
	  //printf("%d: %d, ",i*j+j, AC_RAW_DATA[i][j]);
	}
	for(int k = 0; k < infoBuffersize; k++){
	  acdcData[AC_adr].AC_INFO[i][k] = buffer[usb_read_offset_flag + 
				 (i+1)*psec_buffersize
				 +  (i*infoBuffersize) + k];
	  //printf("%x %d\n", AC_INFO[i][k],k);
	}
      }   
    }

    catch(...){
      fprintf(stderr, "Please connect the board. [DEFAULT exception]\n");
      return 1;
    }

    return 0;
  }
}

int SuMo::dump_data(){
  
  for(int i = 0; i < 4; i++){
    //  if(DC_ACTIVE[i] == false){
    //  continue;
    //}
    //else{ */
      read_AC(false, 1,i);
      //}
  }
  manage_cc_fifo(1);
  return 0;
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
   
  acdcData[aa].BIN_COUNT_RISE =       AC_INFO[0][9] & 0x0F;
  acdcData[aa].BIN_COUNT_FALL =       AC_INFO[0][9] & 0xF0;
  acdcData[aa].SELF_TRIG_SETTINGS =   AC_INFO[1][9];
  acdcData[aa].EVENT_COUNT =          AC_INFO[2][9]  | AC_INFO[2][9]  << 16;
  acdcData[aa].REG_SELF_TRIG[0] =     AC_INFO[0][10] | AC_INFO[1][10] << 16;
  acdcData[aa].REG_SELF_TRIG[1] =     AC_INFO[2][10] | AC_INFO[3][10] << 16;
  acdcData[aa].REG_SELF_TRIG[2] =     AC_INFO[4][10] | AC_INFO[0][11] << 16;
  acdcData[aa].REG_SELF_TRIG[3] =     AC_INFO[1][11] | AC_INFO[2][11] << 16;
  acdcData[aa].SELF_TRIG_MASK =       AC_INFO[3][11] | AC_INFO[4][11] << 16;
  acdcData[aa].LAST_AC_INSTRUCT =     AC_INFO[0][12] | AC_INFO[1][12] << 16;
  acdcData[aa].LAST_LAST_AC_INSTRUCT= AC_INFO[2][12] | AC_INFO[3][12] << 16;

  acdcData[aa].TRIG_EN =              acdcData[aa].SELF_TRIG_SETTINGS & 0x1;
  acdcData[aa].TRIG_WAIT_FOR_SYS =    acdcData[aa].SELF_TRIG_SETTINGS & 0x2;
  acdcData[aa].TRIG_RATE_ONLY =       acdcData[aa].SELF_TRIG_SETTINGS & 0x4;
  acdcData[aa].TRIG_SIGN =            acdcData[aa].SELF_TRIG_SETTINGS & 0x8;
  
  //for (int i =0; i<AC_CHANNELS; i++){
  //  acdcData[aa].SELF_TRIG_RATE_COUNT;
  //}

  if(PRINT){
    cout << std::fixed;
    cout << std::setprecision(2);
    cout << "--------" << endl;
    cout << "event count: " << acdcData[aa].EVENT_COUNT;
    //cout << " LAST TRIG TIMESTAMP: " << acdcData[aa].TIMESTAMP;
    cout << endl;
    cout << "last instructions: 0x" <<  std::hex << acdcData[aa].LAST_AC_INSTRUCT 
	 << ", 0x" << acdcData[aa].LAST_LAST_AC_INSTRUCT << endl;
    cout << "registered self-trig bits: 0x" << hex << acdcData[aa].REG_SELF_TRIG[0] 
	 <<", 0x"                           << hex << acdcData[aa].REG_SELF_TRIG[1]
	 <<", 0x"                           << hex << acdcData[aa].REG_SELF_TRIG[2]
         <<", 0x"                           << hex << acdcData[aa].REG_SELF_TRIG[3] 
	 << endl;
    cout << "self-trig mask: 0x" << hex << acdcData[aa].SELF_TRIG_MASK << endl;
    cout << "self-trig settings: 0x" << hex << acdcData[aa].SELF_TRIG_SETTINGS << endl;  
    cout << "trig_sign: " << acdcData[aa].TRIG_SIGN 
	 << ", wait_for_sys: " << acdcData[aa].TRIG_WAIT_FOR_SYS
	 << ", rate_only: " << acdcData[aa].TRIG_RATE_ONLY
	 << ", EN: " << acdcData[aa].TRIG_EN 
	 << endl;
    cout << "bin count rise edge: " << acdcData[aa].BIN_COUNT_RISE 
	 << ", bin count fall edge: " << acdcData[aa].BIN_COUNT_FALL
         << endl;

    for (int i = 0; i < 5; i++){
      cout << "PSEC:" << i;
      cout << "|ADC clock/trgt:" << acdcData[aa].RO_CNT[i];
      cout << "/" << acdcData[aa].RO_TARGET_CNT[i] << "MHz";
      cout << ",bias:" <<  acdcData[aa].RO_DAC_VALUE[i] <<"mV";
      cout << "|Ped:"<<  acdcData[aa].VBIAS[i] << "mV";
      cout << "|Trig:"<< acdcData[aa].TRIGGER_THRESHOLD[i] << "mV";
      cout << endl;
    }
    cout << endl;
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
