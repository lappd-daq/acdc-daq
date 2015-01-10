#include "SuMo.h"

const unsigned int boardAdrOffset = 25;

void SuMo::createUSBHandles()
{
  usb.createHandles();
  if(mode == USB2x) usb2.createHandles();
}
void SuMo::closeUSBHandles()
{
  usb.freeHandles();
  if(mode == USB2x) usb2.freeHandles();
}

void SuMo::sync_usb(bool SYNC)
{
  createUSBHandles();
  if(SYNC != false)
    {   //enable USB_SYNC
      usb.sendData((unsigned int)0x000F0001);    
      if(mode == USB2x) usb2.sendData((unsigned int)0x000F0001);
    }
  else
    {  //disable USB_SYNC
      usb.sendData((unsigned int)0x000F0000);
      if(mode == USB2x) usb2.sendData((unsigned int)0x000F0000);
      
    }
  closeUSBHandles();
}
/*
 *
 */
void SuMo::software_trigger(unsigned int SOFT_TRIG_MASK)
{
    usb.createHandles();
    //usb.sendData((unsigned int)0x000E0000);//software trigger
    const unsigned int hi_cmd = 0x000E0000;    
    unsigned int send_word = hi_cmd | SOFT_TRIG_MASK;
    usb.sendData(send_word);
    //printf("sent software trigger\n");
    usb.freeHandles();
}
void SuMo::software_trigger_slaveDevice(unsigned int SOFT_TRIG_MASK)
{
    usb2.createHandles();
    //usb.sendData((unsigned int)0x000E0000);//software trigger
    const unsigned int hi_cmd = 0x000E0000;    
    unsigned int send_word = hi_cmd | SOFT_TRIG_MASK;
    usb2.sendData(send_word);
    //printf("sent software trigger\n");
    usb2.freeHandles();
}
/*
 *
 */
void SuMo::align_lvds()
{
  createUSBHandles();
  usb.sendData((unsigned int)0x000D0000);  //toggle align process
  if(mode == USB2x) usb2.sendData((unsigned int)0x000D0000);
  closeUSBHandles();
}
/*
 *
 */
 void SuMo::toggle_LED(bool EN)
{
  createUSBHandles();
  if(EN != false){
    usb.sendData((unsigned int)0x000A0001);
    if(mode == USB2x) usb2.sendData((unsigned int)0x000A0001);
  } 
  else{
    usb.sendData((unsigned int)0x000A0000);
    if(mode == USB2x) usb2.sendData((unsigned int)0x000A0000);
  }
  closeUSBHandles();
}
/*
 *
 */
void SuMo::toggle_CAL(bool EN, int device)
{
  createUSBHandles();
 
  unsigned int send_word = 0x00020000;
  
  if(EN != false){							

    send_word = send_word | 15 << boardAdrOffset | 0x7FFF;
    if(device == 0)                usb.sendData(send_word);
    if(device == 1 && mode==USB2x) usb2.sendData(send_word);

  }
  else{
    send_word = send_word | 15 << boardAdrOffset;
    if(device == 0)                usb.sendData(send_word);
    if(device == 1 && mode==USB2x) usb2.sendData(send_word);
  }
  closeUSBHandles();
}

void SuMo::set_pedestal_value(  unsigned int PED_VALUE, 
				unsigned int boardAdr, 
				int device)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00030000;    
  unsigned int send_word = hi_cmd | PED_VALUE | boardAdr << boardAdrOffset;
  
  if(device == 0)                 usb.sendData(send_word);
  if(device == 1 && mode==USB2x)  usb2.sendData(send_word);
  
  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_self_trigger(     bool ENABLE_TRIG, 
				 bool SYS_TRIG_OPTION, 
				 bool RATE_ONLY, 
				 bool TRIG_SIGN, 
				 bool USE_BOARD_SMA_TRIG,
				 unsigned int boardAdr, 
				 int device)
{
    const unsigned int hi_cmd = 0x00070000;   
    unsigned int send_word = hi_cmd | USE_BOARD_SMA_TRIG << 4 | TRIG_SIGN << 3 | RATE_ONLY << 2 
                                    | SYS_TRIG_OPTION << 1 | ENABLE_TRIG | boardAdr << boardAdrOffset;
    //printf("%i\n", send_word);
    createUSBHandles();

    if(device == 0)                  usb.sendData((unsigned int)send_word);
    if(device == 1 && mode == USB2x) usb2.sendData((unsigned int)send_word);
    
    closeUSBHandles();
}
/*
 *
 */
void SuMo::set_self_trigger_mask(int mask, bool HiLo, unsigned int boardAdr, int device)
{
  unsigned int hi_cmd;
  if(HiLo) hi_cmd = 0x00068000; 
  else     hi_cmd = 0x00060000; 
  
  unsigned int send_word = hi_cmd | mask | boardAdr << boardAdrOffset;
  
  createUSBHandles();
  if(device == 0)                  usb.sendData((unsigned int)send_word);
  if(device == 1 && mode == USB2x) usb2.sendData((unsigned int)send_word);
  
  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_dll_vdd(unsigned int VALUE, unsigned int boardAdr, int device)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00010000;    
  unsigned int send_word = hi_cmd | VALUE | boardAdr << boardAdrOffset;
  
  if(device == 0)                usb.sendData(send_word);
  if(device == 1 && mode==USB2x) usb2.sendData(send_word);
  
  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_trig_threshold(unsigned int TRIG_VALUE, unsigned int boardAdr, int device)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00080000;    
  const unsigned int mask = 31;  //chip mask
  unsigned int send_word = hi_cmd | TRIG_VALUE | mask << 20 | boardAdr << boardAdrOffset;

  if(device == 0)                usb.sendData(send_word);
  if(device == 1 && mode==USB2x) usb2.sendData(send_word);

  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_usb_read_mode(unsigned int READ_MODE)
{
  usb.createHandles();
  const unsigned int hi_cmd = 0x000C0000;    
  unsigned int send_word = hi_cmd | READ_MODE | 15 << boardAdrOffset; 
  usb.sendData(send_word);
  usb.freeHandles();
  
}
void SuMo::set_usb_read_mode_slaveDevice(unsigned int READ_MODE)
{
  usb2.createHandles();
  const unsigned int hi_cmd = 0x000C0000;    
  unsigned int send_word = hi_cmd | READ_MODE | 15 << boardAdrOffset; 
  usb2.sendData(send_word);
  usb2.freeHandles();

}
/*
 *
 */
void SuMo::set_ro_target_count(unsigned int TARGET_RO_COUNT)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00090000;    
  unsigned int send_word = hi_cmd | TARGET_RO_COUNT | 15 << boardAdrOffset; 
  usb.sendData(send_word);
  if(mode==USB2x) usb2.sendData(send_word);

  closeUSBHandles();
}
/*
 *
 */
/* various reset commands over USB */
void SuMo::reset_dll()
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00041000;  
  const unsigned int mask = 31; //chip mask
  unsigned int send_word = hi_cmd |  mask << 20 | 15 << boardAdrOffset;
  
  usb.sendData(send_word); //dll reset pulse
  if(mode == USB2x) usb2.sendData(send_word);
  
  closeUSBHandles();
}
void SuMo::reset_self_trigger(unsigned int boardAdr, int device)
{
  createUSBHandles();
  unsigned int send_word = 0x00042000 | boardAdr << boardAdrOffset;
  
  if(device == 0)                  usb.sendData(send_word);
  if(device == 1 && mode == USB2x) usb2.sendData(send_word);

  closeUSBHandles();

}
void SuMo::reset_time_stamp()
{
  createUSBHandles();
  unsigned int send_word = 0x00043000 | 15 << boardAdrOffset;
  
  usb.sendData(send_word);
  if(mode == USB2x) usb2.sendData(send_word);

  closeUSBHandles();
}
void SuMo::reset_acdc()
{
  usb.createHandles();
  unsigned int send_word = 0x0004F000 | 15 << boardAdrOffset;
  
  usb.sendData(send_word);
  if(mode == USB2x) usb2.sendData(send_word);

  closeUSBHandles();
}
/*
 *
 */ 
void SuMo::manage_cc_fifo(bool VALUE)
{
  usb.createHandles();
  const unsigned int hi_cmd = 0x000B0000;
  unsigned int send_word = hi_cmd | VALUE;
  usb.sendData(send_word);
  usb.freeHandles();
}

void SuMo::manage_cc_fifo_slaveDevice(bool VALUE)
{
  usb2.createHandles();
  const unsigned int hi_cmd = 0x000B0000;
  unsigned int send_word = hi_cmd | VALUE;
  usb2.sendData(send_word);
  usb2.freeHandles();
}

