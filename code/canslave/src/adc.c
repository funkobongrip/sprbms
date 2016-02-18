#include "main.h"

volatile uint32_t ADCRounded[TEMPCOUNT+1];

volatile uint32_t TempADC[8] = 	{
									TEMPR_ADC,
									TEMP1_ADC,
									TEMP2_ADC,
									TEMP3_ADC,
									TEMP4_ADC,
									TEMP5_ADC,
									TEMP6_ADC,
									TEMP7_ADC
									};

volatile uint32_t *ADCPIO[8] = 	{
									&(LPC_IOCON->R_PIO0_11),
									&(LPC_IOCON->R_PIO1_0),
									&(LPC_IOCON->R_PIO1_1),
									&(LPC_IOCON->R_PIO1_2),
									&(LPC_IOCON->SWDIO_PIO1_3),
									&(LPC_IOCON->PIO1_4),
									&(LPC_IOCON->PIO1_10),
									&(LPC_IOCON->PIO1_11)
									};

volatile uint32_t ADCPIOConf[8] = 	{
									0x2, //0_11
									0x2, //1_0
									0x2, //1_1
									0x2, //1_2
									0x2, //1_3
									0x1, //1_4
									0x1, //1_10
									0x1  //1_11
									};

void ADCInit()
{
	uint32_t i;
	
	/* Disable Power down bit to the ADC block. */  
	LPC_SYSCON->PDRUNCFG &= ~(0x1<<4);

	/* Enable AHB clock to the ADC. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);

	// select adc clock
	LPC_ADC->CR = (((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/ADC_CLK)-1)<<8;

	LPC_ADC->INTEN = 0x0;

	uint32_t highchan = 0;
	for(i=0; i<=TEMPCOUNT ;i++)
	{
		// get adc channel number for temp i (0 is R)
		uint32_t adcchan = TempADC[i];
		
		ADCRounded[i] = 0;
		
		// set gpio to AD mode
		*(ADCPIO[adcchan]) = ADCPIOConf[adcchan];
		
		// add AD channel for conversion
		LPC_ADC->CR |= (1 << adcchan);

		if( adcchan > highchan )
		{
			highchan = adcchan;
		}
	}

	// enable interrupt for highest channel
	LPC_ADC->INTEN = (1<<highchan);

	NVIC_EnableIRQ(ADC_IRQn);

	// enable burst mode (and start conversion)
	LPC_ADC->CR |= (1<<16);

	return;
}

void ADC_IRQHandler(void)
{
	uint32_t regVal, temp;

	// Read ADC will clear the interrupt
	regVal = LPC_ADC->STAT;
	
	for(temp=0; temp<=TEMPCOUNT ;temp++)
	{
		// get adc channel number for temp i (0 is R)
		uint32_t adcchan = TempADC[temp];
		
		if( regVal & (1<<adcchan) )
		{
			uint32_t adcval = (LPC_ADC->DR[adcchan] & 0xFFC0) >> 6;
			// conversion done for adcchan
			ADCRounded[temp] -= ADCRounded[temp] >> 6;
			ADCRounded[temp] += adcval;
		}
	}
	
	temp = ADCRounded[1] >> 6;
	if( temp >= 1024 )
		msg_temp.data[0] = 0;
	else
		msg_temp.data[0] = ntc_vals[temp];;

	temp = ADCRounded[2] >> 6;
	if( temp >= 1024 )
		msg_temp.data[1] = 0;
	else
		msg_temp.data[1] = ntc_vals[temp];

	temp = ADCRounded[3] >> 6;
	if( temp >= 1024 )
		msg_temp.data[2] = 0;
	else
		msg_temp.data[2] = ntc_vals[temp];

	temp = ADCRounded[4] >> 6;
	if( temp >= 1024 )
		msg_temp.data[3] = 0;
	else
		msg_temp.data[3] = ntc_vals[temp];
	
	// TODO: calculate diode forward drop
	temp = ADCRounded[0] >> 6;
	msg_temp.data[5] = 0;//ntc_vals[temp]; TODO: diode_vals?!
	
	return;
}
