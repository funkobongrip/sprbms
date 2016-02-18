/*
 * Copyright (c) 2012 David Brandt
 *  All rights reserved.
 *
 * Licensed under: CC BY-NC-SA 3.0 (full license in file LICENSE)
 * 
 * You are free:
 * - to Share - to copy, distribute and transmit the work
 * - to Remix - to adapt the work
 * 
 * Under the following conditions:
 * - Attribution - You must attribute the work in the manner specified by
 *   the author or licensor (but not in any way that suggests that
 *   they endorse you or your use of the work).
 * 
 * - Noncommercial - You may not use this work for commercial purposes.
 * 
 * - Share Alike - If you alter, transform, or build upon this work, you may
 *   distribute the resulting work only under the same or similar license
 *   to this one.
 */
 
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "config.h"
#include "adc.h"
#include "mcan.h"
#include "pec.h"
#include "slave.h"
#include "spi.h"
#include "spi_adc.h"
#include "uart.h"
#include "timer.h"
#include "charger.h"
#include "relai.h"
#include "intermediate.h"
#include "balancing.h"

volatile unsigned char vcc_volt = 0x78, relai_volt = 0x78;
volatile unsigned int vcc_volt_avg = 0x7800, relai_volt_avg = 0x7800;
volatile unsigned char adc_wait_timer = 0;

void adc_init(void)
{
	ADMUX = ADC_CHANNEL_VCC;
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	//DIDR0 = (1<<ADC2D) | (1<<ADC1D) | (1<<ADC0D);
}

void adc_start(unsigned char chan)
{
	ADMUX = chan;
	ADCSRA |= (1<<ADSC);
}

// reference: 2.56V, 10 bit, 0,0025V per 1
// x * 0,0025V = v
// divider for voltage: 4700ohm / 680ohm
// U2 = (R2 * U) / (R1 + R2)
// U = (U2 * (R1 + R2)) / R2
// U =(v * (4700 + 680)) / 680
// U = v * 5380 / 680
// U = v * 7,912
// U = (x * 2,5mV) * 7,912
// U = x * 19,78mV
// U = x * 1978mV / 100
// We want an 8 bit value representing 100mV each
// so wie divide the result with 100
// U = x * 1978mV / 10000

void adc_loop(void)
{
	static unsigned char chan = ADC_CHANNEL_RELAI;
	uint32_t adc_tmp;
	
	if( (ADCSRA & (1<<ADSC)) == 0 && adc_wait_timer == 0 )
	{
		adc_tmp = ADCL;
		adc_tmp |= (ADCH << 8);
		
		if( chan == ADC_CHANNEL_RELAI )
		{
			adc_tmp *= 1978;
			adc_tmp /= 10000;
			relai_volt_avg -= ((relai_volt_avg >> 8) & 0xFF);
			relai_volt_avg += adc_tmp;
			relai_volt = ((relai_volt_avg >> 8) & 0xFF);
			
			chan = ADC_CHANNEL_VCC;
			adc_start(chan);
			adc_wait_timer = ADC_WAIT_TIME;
		}
		else if( chan == ADC_CHANNEL_VCC )
		{
			adc_tmp *= 1978;
			adc_tmp /= 10000;
			vcc_volt_avg -= ((vcc_volt_avg >> 8) & 0xFF);
			vcc_volt_avg += adc_tmp;
			vcc_volt = ((vcc_volt_avg >> 8) & 0xFF);
			
			chan = ADC_CHANNEL_TEMP;
			adc_start(chan);
			adc_wait_timer = ADC_WAIT_TIME;
		}
		else if( chan == ADC_CHANNEL_TEMP )
		{
			chan = ADC_CHANNEL_RELAI;
			adc_start(chan);
			adc_wait_timer = ADC_WAIT_TIME;
		}
	}
}
