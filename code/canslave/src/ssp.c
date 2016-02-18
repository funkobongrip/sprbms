#include "main.h"

volatile uint32_t interruptRxStat1 = 0;
volatile uint32_t interruptOverRunStat1 = 0;
volatile uint32_t interruptRxTimeoutStat1 = 0;

volatile uint8_t SSPBuf[32];
volatile uint8_t SSPState = 0, SSPCMDState = 0;

const struct ssp_state_struct SSPStates[] =
{
		{.cmd = 0x01, .pec = 0xC7, .rlen = 0, .wlen = 6}, // write configuration
		{.cmd = 0x10, .pec = 0xB0, .rlen = 0, .wlen = 0}, // Start cell voltage conversion for all 12 cells and poll
		{.cmd = 0x30, .pec = 0x50, .rlen = 0, .wlen = 0}, // Start temperature conversion and poll
		{.cmd = 0x52, .pec = 0x79, .rlen = 0, .wlen = 0}, // Start diagnose and poll
		{.cmd = 0x02, .pec = 0xCE, .rlen = 6, .wlen = 0}, // read configuration
		{.cmd = 0x06, .pec = 0xD2, .rlen = 6, .wlen = 0}, // read cells 1-4
#if CELLCOUNT > 4
		{.cmd = 0x08, .pec = 0xF8, .rlen = 6, .wlen = 0}, // read cells 5-8
#endif
#if CELLCOUNT > 8
		{.cmd = 0x0A, .pec = 0xF6, .rlen = 6, .wlen = 0}, // read cells 9-12
#endif
		{.cmd = 0x0E, .pec = 0xEA, .rlen = 5, .wlen = 0}, // read temperature register
		{.cmd = 0x54, .pec = 0x6B, .rlen = 2, .wlen = 0}, // read diagnosis register
		{.cmd = 0, .pec = 0, .rlen = 0, .wlen = 0}
};

void SSP_Init()
{
	uint8_t i, Dummy=Dummy;

	LPC_SYSCON->PRESETCTRL |= (1<<0);
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);
	// device clock by 2
	LPC_SYSCON->SSP0CLKDIV = 0x02;
	// MISO0 for SSP
	LPC_IOCON->PIO0_8 &= ~0x07;
	LPC_IOCON->PIO0_8 |= 0x01;
	// MOSI0 for SSP
	LPC_IOCON->PIO0_9 &= ~0x07;
	LPC_IOCON->PIO0_9 |= 0x01;
	// SCK0 for SSP
	LPC_IOCON->SCK_LOC &= ~0x07;
	LPC_IOCON->SCK_LOC = 0x02;
	LPC_IOCON->PIO0_6 &= ~0x07;
	LPC_IOCON->PIO0_6 |= 0x02;
	// SSEL0 for SSP, standard GPIO
	LPC_IOCON->PIO0_2 &= ~0x07;
	SetGPIOOut(0, 2);
	SetGPIOBit(0, 2);

	/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP0->CR0 = 0x07C7;

	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP0->CPSR = 0x24;

	for ( i = 0; i < FIFOSIZE; i++ )
	{
		Dummy = LPC_SSP0->DR;		/* clear the RxFIFO */
	}

	/* Enable the SSP Interrupt */
	//NVIC_EnableIRQ(SSP0_IRQn);
	
	// Device select as master, SSP Enabled
	LPC_SSP0->CR1 = SSPCR1_SSE;

	return;
}

void SSPBulkStart(volatile uint8_t *buf, uint32_t length)
{
    uint32_t i;
    SSPCSLow();
    for ( i = 0; i < length; i++ )
    {
        LPC_SSP0->DR = buf[i];
    }
    return;
}

void SSPBulkStop(volatile uint8_t *buf, uint32_t length)
{
    uint32_t i;
    SSPCSHigh();
    for ( i = 0; i < length; i++ )
    {
        buf[i] = LPC_SSP0->DR;
    }
    return;
}

void SSPUpdatePec(uint8_t *pec, uint8_t data)
{
	uint8_t i;
  uint8_t in[3];
  uint8_t tmp_pec;

  tmp_pec = *pec;

#define pec_din(i) ((data >> i) & 0x01)
#define pec_val(i) ((tmp_pec >> i) & 0x01)

  for(i=8; i>0 ;)
  {
    in[0] = pec_din((--i)) ^ pec_val(7);
    in[1] = pec_val(0) ^ in[0];
    in[2] = pec_val(1) ^ in[0];

    *pec = 0;
    *pec |= pec_val(6) << 7;
    *pec |= pec_val(5) << 6;
    *pec |= pec_val(4) << 5;
    *pec |= pec_val(3) << 4;
    *pec |= pec_val(2) << 3;
    *pec |= (in[2] & 0x01) << 2;
    *pec |= (in[1] & 0x01) << 1;
    *pec |= (in[0] & 0x01) << 0;

    tmp_pec = *pec;
  }
  *pec = tmp_pec;
}

void SSPSendCommand(void)
{
	// start sending command.
	SSPBuf[0] = SSPStates[SSPState].cmd;
	SSPBuf[1] = SSPStates[SSPState].pec;
	// cmd,pec,writelen+readlen,pec
	SSPBulkStart(SSPBuf, 2);
	SSPCMDState = 1;
}

void SSPDoData(void)
{
	uint8_t Dummy = Dummy, pec = 0x41, i;
	Dummy = LPC_SSP0->DR;
	Dummy = LPC_SSP0->DR;

	if( SSPStates[SSPState].wlen > 0 )
	{
		SlaveDataWrite(SSPStates[SSPState], (uint8_t *)SSPBuf);
		for(i=0; i<SSPStates[SSPState].wlen; i++)
		{
			SSPUpdatePec(&pec, SSPBuf[i]);
		}
		SSPBuf[SSPStates[SSPState].wlen] = pec;
	}
	SSPBulkStart(SSPBuf, SSPStates[SSPState].wlen + SSPStates[SSPState].rlen + 1);
	SSPCMDState = 0;
}

// TODO: when no slave data is received within time x, yield error

void SSPLoop(void)
{
	uint8_t pec = 0x41, i;

	// ssp is busy? return.
	if( (LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE )
	{
		return;
	}

	// we are polling, check if SDO is high
	if( SSPStates[SSPState].rlen == 0 && SSPStates[SSPState].wlen == 0 && GetGPIOBit(0, 8) == 0 )
	{
		// SDO is high, we need to wait some more.
		static unsigned int bla = 0;
		bla = SSPState;
		SSPState = SSPBuf[0];
		SSPState = bla;
		return;
	}

	if( SSPCMDState )
	{
		SSPDoData();
		return;
	}

	SSPBulkStop(SSPBuf, SSPStates[SSPState].wlen + SSPStates[SSPState].rlen + 1);

	if( SSPStates[SSPState].rlen > 0 )
	{
		for(i=0; i<SSPStates[SSPState].rlen ;i++)
		{
			SSPUpdatePec(&pec, SSPBuf[i]);
		}

		if( SSPBuf[SSPStates[SSPState].rlen] != pec )
		{
			// TODO: error! do something here.
			SSPSendCommand();
			return;
		}
		else
		{
			// data transmitted ok, handover to slave function
			SlaveDataRead(SSPStates[SSPState], (uint8_t *)SSPBuf);
		}
	}

	SSPState++;
	if( SSPStates[SSPState].cmd == 0 && SSPStates[SSPState].rlen == 0 && SSPStates[SSPState].wlen == 0 )
	{
		SSPState = 0;
	}

	SSPSendCommand();
}
