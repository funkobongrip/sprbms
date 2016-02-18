#include "main.h"

volatile uint8_t slaveStatus = 0x00;
volatile uint8_t slaveID = 0x0;

volatile uint32_t TimeTick = 0;

void SysTick_Handler(void)
{
	TimeTick++;
	CAN1msTimer();
}

void setSlaveState(uint8_t err)
{
	if( !err )
	{
		// clear balancing flag
		slaveStatus &= ~SLAVE_BALANCING;
	}
	else if( err == SLAVE_BALANCING )
	{
		// this is a notice only
		slaveStatus |= err;
	}
	else
	{
		// we have some sort of error
		BalancingStatus = 0;
		slaveStatus &= ~SLAVE_BALANCING;
		slaveStatus |= err;
		slaveStatus |= SLAVE_ERR;
	}

	msg_status.data[0] = slaveStatus;
	
	ClrGPIOBits(2, 10);

	CANStartSend();
}

void delaySysTick(uint32_t tick)
{
	uint32_t timetick;

	timetick = TimeTick;
	while ((TimeTick - timetick) < tick)
		__WFI();
}

int main(void)
{
	// set System Tick Timer to 1ms
	SysTick_Config(SystemCoreClock / 1000);
	InitGPIO();
	slaveID = getSlaveID();
	SSP_Init();
	CANInit();
	ADCInit();

	// Start Measuring cycle
	SSPSendCommand();
	while(1)
	{
		SSPLoop();
	}
	return 1;
}
