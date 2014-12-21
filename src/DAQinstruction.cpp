#include "SuMo.h"

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
void SuMo::set_self_trigger(bool ENABLE_TRIG, bool SYS_TRIG_OPTION, bool RATE_ONLY, bool TRIG_SIGN)
{
    const unsigned int hi_cmd = 0x00070000;   
    unsigned int send_word = hi_cmd | TRIG_SIGN << 3 | RATE_ONLY << 2 | SYS_TRIG_OPTION << 1 | ENABLE_TRIG;
    //printf("%i\n", send_word);
    createUSBHandles();

    usb.sendData((unsigned int)send_word);
    if(mode == USB2x) usb2.sendData((unsigned int)send_word);
    closeUSBHandles();
}
/*
 *
 */
void SuMo::set_self_trigger_mask(int mask, bool HiLo)
{
  unsigned int hi_cmd;
  if(HiLo)
    hi_cmd = 0x00068000; 
  else
    hi_cmd = 0x00060000; 
  
  unsigned int send_word = hi_cmd | mask;
  
  createUSBHandles();
  usb.sendData((unsigned int)send_word);
  if(mode == USB2x) usb2.sendData((unsigned int)send_word);
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
void SuMo::toggle_CAL(bool EN)
{
  createUSBHandles();
  if(EN != false){
    usb.sendData((unsigned int)0x00020001);
    if(mode==USB2x) usb2.sendData((unsigned int)0x00020001);

  }
  else{
    usb.sendData((unsigned int)0x00020000);
    if(mode==USB2x) usb2.sendData((unsigned int)0x00020000);
  }
  closeUSBHandles();
}

void SuMo::set_pedestal_value(unsigned int PED_VALUE)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00030000;    
  const unsigned int mask = 0x0;
  unsigned int send_word = hi_cmd | PED_VALUE | mask << 23;
  
  usb.sendData(send_word);
  if(mode==USB2x)  usb2.sendData(send_word);
  
  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_dll_vdd(unsigned int VALUE)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00010000;    
  const unsigned int mask = 0x0;
  unsigned int send_word = hi_cmd | VALUE | mask << 23;
  usb.sendData(send_word);
  if(mode==USB2x) usb2.sendData(send_word);
  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_trig_threshold(unsigned int TRIG_VALUE)
{
  createUSBHandles();
  const unsigned int hi_cmd = 0x00080000;    
  const unsigned int mask = 31;
  unsigned int send_word = hi_cmd | TRIG_VALUE | mask << 23;

  usb.sendData(send_word);
  if(mode==USB2x) usb2.sendData(send_word);

  closeUSBHandles();
}
/*
 *
 */
void SuMo::set_usb_read_mode(unsigned int READ_MODE)
{
  usb.createHandles();
  const unsigned int hi_cmd = 0x000C0000;    
  unsigned int send_word = hi_cmd | READ_MODE;
  usb.sendData(send_word);
  usb.freeHandles();
  
}
void SuMo::set_usb_read_mode_slaveDevice(unsigned int READ_MODE)
{
  usb2.createHandles();
  const unsigned int hi_cmd = 0x000C0000;    
  unsigned int send_word = hi_cmd | READ_MODE;
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
  unsigned int send_word = hi_cmd | TARGET_RO_COUNT;
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
  const unsigned int mask = 31;
  unsigned int send_word = hi_cmd |  mask << 23;
  usb.sendData(send_word); //dll reset pulse
  if(mode == USB2x) usb2.sendData(send_word);
  closeUSBHandles();
}
void SuMo::reset_self_trigger()
{
  createUSBHandles();

  usb.sendData((unsigned int)0x00042000);
  if(mode == USB2x) usb2.sendData((unsigned int)0x00042000);

  closeUSBHandles();

}
void SuMo::reset_time_stamp()
{
  createUSBHandles();

  usb.sendData((unsigned int)0x00043000);
  if(mode == USB2x) usb2.sendData((unsigned int)0x00043000);

  closeUSBHandles();
}
void SuMo::reset_acdc()
{
  usb.createHandles();

  usb.sendData((unsigned int)0x0004F000);
  if(mode == USB2x) usb2.sendData((unsigned int)0x0004F000);

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

