/////////////////////////////////////////////////////
//  file: PSEC4_EVAL_USB.cxx
//
//  USB interface definitions to PSEC-4 6-channel DAQ
//
//  Revision History:
//          01/2012 updated
//
//  Author: ejo
//////////////////////////////////////////////////////
#include "SuMo.h"

void SuMo::sync_usb(bool SYNC)
{
    usb.createHandles();
    if(SYNC != false)
    {
        usb.sendData((unsigned int)0x000F0001);//enable USB_SYNC
    }
    else
    {
        usb.sendData((unsigned int)0x000F0000);//disable USB_SYNC
    }
    usb.freeHandles();
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
/*
 *
 */
void SuMo::align_lvds()
{
    usb.createHandles();
    usb.sendData((unsigned int)0x000D0000);//disable align process
    usb.freeHandles();
}
/*
 *
 */
void SuMo::set_self_trigger(bool ENABLE_TRIG, bool SYS_TRIG_OPTION, bool RATE_ONLY, bool TRIG_SIGN)
{
    const unsigned int hi_cmd = 0x00070000;
    unsigned int send_word = hi_cmd | TRIG_SIGN << 3 | RATE_ONLY << 2 | SYS_TRIG_OPTION << 1 | ENABLE_TRIG;
    //printf("%i\n", send_word);
    usb.createHandles();
    usb.sendData((unsigned int)send_word);
    usb.freeHandles();
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
  usb.createHandles();
  usb.sendData((unsigned int)send_word);
  usb.freeHandles();
}
/*
 *
 */
 void SuMo::toggle_LED(bool EN)
{
    usb.createHandles();
    if(EN != false)
      usb.sendData((unsigned int)0x000A0001);
    else
      usb.sendData((unsigned int)0x000A0000);
    usb.freeHandles();
}
/*
 *
 */
void SuMo::toggle_CAL(bool EN)
{
  usb.createHandles();
  if(EN != false)
    usb.sendData((unsigned int)0x00020001);
  else
    usb.sendData((unsigned int)0x00020000);
  usb.freeHandles();
}

void SuMo::set_pedestal_value(unsigned int PED_VALUE)
{
    usb.createHandles();
    const unsigned int hi_cmd = 0x00030000;
    const unsigned int mask = 0x0;
    unsigned int send_word = hi_cmd | PED_VALUE | mask << 23;
    usb.sendData(send_word);
    usb.freeHandles();
}
/*
 *
 */
void SuMo::set_dll_vdd(unsigned int VALUE)
{
    usb.createHandles();
    const unsigned int hi_cmd = 0x00010000;
    const unsigned int mask = 0x0;
    unsigned int send_word = hi_cmd | VALUE | mask << 23;
    usb.sendData(send_word);
    usb.freeHandles();
}
/*
 *
 */
void SuMo::set_trig_threshold(unsigned int TRIG_VALUE)
{
    usb.createHandles();
    const unsigned int hi_cmd = 0x00080000;
    const unsigned int mask = 31;
    unsigned int send_word = hi_cmd | TRIG_VALUE | mask << 23;
    usb.sendData(send_word);
    usb.freeHandles();
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
/*
 *
 */
void SuMo::set_ro_target_count(unsigned int TARGET_RO_COUNT)
{
    usb.createHandles();
    const unsigned int hi_cmd = 0x00090000;
    unsigned int send_word = hi_cmd | TARGET_RO_COUNT;
    cout << send_word;
    usb.sendData(send_word);
    usb.freeHandles();
}
/*
 *
 */
/* various reset commands over USB */
void SuMo::reset_dll()
{
    usb.createHandles();
    const unsigned int hi_cmd = 0x00041000;
    const unsigned int mask = 31;
    unsigned int send_word = hi_cmd |  mask << 23;
    usb.sendData(send_word); //dll reset pulse
    usb.freeHandles();
}
void SuMo::reset_self_trigger()
{
    usb.createHandles();
    usb.sendData((unsigned int)0x00042000);
    usb.freeHandles();
}
void SuMo::reset_time_stamp()
{
    usb.createHandles();
    usb.sendData((unsigned int)0x00043000);
    usb.freeHandles();
}
void SuMo::reset_acdc()
{
    usb.createHandles();
    usb.sendData((unsigned int)0x0004F000);
    usb.freeHandles();
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
/*
 *
 */
int SuMo::check_usb()
{
  if(usb.createHandles() != stdUSB::SUCCEED)
    return 1;
  else{
    usb.freeHandles();
    return 0;
  }
}
