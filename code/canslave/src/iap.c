#include "main.h"

uint32_t slaveUIDTable[][5] = {
	{0, 0x11110b31, 0x53310000, 0x52032085, 4}, //2
	{0, 0xe0e0023, 0x53310000, 0x5202e802, 6}, //9
	{0, 0xe0ef822, 0x53310000, 0x5202eb3a, 7}, //7
	{0, 0x11110431, 0x53310000, 0x52031e52, 5}, //6
	{0, 0x11111131, 0x53310000, 0x52032085, 1}, //5
	{0, 0x11110531, 0x53310000, 0x52031e52, 3}, //8
	{0, 0xe0eff23, 0x53310000, 0x5202e802, 0}, //nix
	{0, 0xe0e0523, 0x53310000, 0x5202e55d, 2}, //3
	{0x0, 0x0, 0x0, 0x0, 0}
};

// This data must be global so it is not read from the stack
typedef void (*IAP)(uint32_t [], uint32_t []);
IAP iap_entry = (IAP)0x1fff1ff1;
uint32_t command[5], result[4];

uint8_t getSlaveID(void)
{
	uint32_t uid[4];
	uint32_t i;
	
	getDeviceUID(uid);
	
		for(i=0; slaveUIDTable[i][0] != 0 ||
			 slaveUIDTable[i][1] != 0 ||
			 slaveUIDTable[i][2] != 0 ||
			 slaveUIDTable[i][3] != 0 ;i++)
	{
		if( slaveUIDTable[i][0] == uid[0] &&
			slaveUIDTable[i][1] == uid[1] && 
			slaveUIDTable[i][2] == uid[2] && 
			slaveUIDTable[i][3] == uid[3] )
		{
			return slaveUIDTable[i][4];
		}
	}
	
	return 0;
}

void getDeviceUID (uint32_t UID_array[4])
{
	uint32_t IAP_command = 58;
	iap_entry(&IAP_command, UID_array);
}

/* This function resets some microcontroller peripherals to reset
   hardware configuration to ensure that the In-System Programming module
   will work properly. ISP is normally called from reset and assumes some reset
   configuration settings for the MCU.
   Some of the peripheral configurations may be redundant in your specific
   project.
*/
void ReinvokeISP(void)
{
  /* make sure 32-bit Timer 1 is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00400;
  /* make sure GPIO clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00040;
  /* make sure IO configuration clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x10000;
  /* make sure AHB clock divider is 1:1 */
  LPC_SYSCON->SYSAHBCLKDIV = 1;

  /* Send Reinvoke ISP command to ISP entry point*/
  command[0] = 57;

  /* Set stack pointer to ROM value (reset default) This must be the last
     piece of code executed before calling ISP, because most C expressions
     and function returns will fail after the stack pointer is changed. */
  __set_MSP(*((uint32_t *)0x1FFF0000)); /* inline asm function */

  /* Invoke ISP. We call "iap_entry" to invoke ISP because the ISP entry is done
     through the same command interface as IAP. */
  iap_entry(command, result);
  // Not supposed to come back!
}
