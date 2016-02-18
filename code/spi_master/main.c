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

volatile unsigned char battery_state = 0;
volatile unsigned char slave_state = 0;
volatile unsigned char relai_state = 0;
volatile unsigned char relai_ext_state = 0;
// warning time
volatile unsigned char warning_time_val = 0;

volatile char error_msg[9];

void warning_time(unsigned char time)
{
	if( time > warning_time_val )
	{
		warning_time_val = time;
	}
	battery_state |= STATE_WARNING;
}

void led_blink_halt(void)
{
	unsigned char i;
	while(1)
	{
		for(i=0; i<100 ;i++)
		{
			__asm__ __volatile__ ("wdr");
			_delay_ms(5);
		}
		TOGGLE(LED_B);
	}
}

void error(char *msg)
{
	unsigned char i;
	standby();
	battery_state = STATE_ERROR;
	
	for(i=0; i<8 ;i++)
	{
		error_msg[i] = msg[i];
	}
	
	RESET_LEDS();
	SET_LED(LED_R);
}

void standby(void)
{
	battery_state = 0;
	relai_state &= ~(RELAI_M_EN | RELAI_P_EN | RELAI_PD_EN);
	RESET_RELAI(RELAIPLUS_CTRL);
	RESET_RELAI(RELAIMINUS_CTRL);
	_delay_ms(10);
	RESET_RELAI(RELAICHARGE_CTRL);
	
#ifdef DCDCCTRL
	RESET(DCDCCTRL);
#endif
	
	RESET_LEDS();
	SET_LED(LED_G);
	
	if( charger_state )
	{
		charger_disable_output();
		charger_disable_output();
		charger_set_idle_curr();
		charger_set_idle_curr();
		charger_state = 0;
	}
}

void status(char *msg)
{
	cmsg.id = CAN_STATUS_ID;
	if( msg != NULL )
	{
		cmsg.length = 8;
		memcpy((void *)cmsg.data, msg, 8);
	}
	else
	{
		cmsg.length = 0;
	}
	can_send_message((const can_t *)&cmsg);
}

void update_states(void)
{
	relai_ext_state = 0;
	
	if( IS_SET(RELAI_VCC) )
		relai_ext_state |= RELAI_EXT_VCC;
	
	if( IS_SET(RELAIPLUS_AND) )
		relai_ext_state |= RELAI_EXT_AND;
		
	if( IS_SET(RELAIPLUS_AND_INV) )
		relai_ext_state |= RELAI_EXT_AND_INV;
	
	if( IS_SET(RELAIPLUS_DELAYED) )
		relai_ext_state |= RELAI_EXT_DELAYED;
		
	if( IS_SET(RELAIPLUS_OUT) )
		relai_ext_state |= RELAI_EXT_OUT;
		
	if( IS_SET(RELAICHARGE) )
		relai_state |= RELAI_PD_CL;
	else
		relai_state &= ~RELAI_PD_CL;
	
	if( IS_SET(RELAIMINUS) )
		relai_state |= RELAI_M_CL;
	else
		relai_state &= ~RELAI_M_CL;
	
	if( IS_SET(RELAIPLUS) )
		relai_state |= RELAI_P_CL;
	else
		relai_state &= ~RELAI_P_CL;
	
	if(! IS_SET(N40V) )
		relai_state |= RELAI_40V;
	else
		relai_state &= ~RELAI_40V;
	
	if(! IS_SET(NPRECHC) )
		relai_state |= RELAI_PRECH;
	else
		relai_state &= ~RELAI_PRECH;
}

static inline void tractive_loop(void)
{
	static unsigned int tmp = 0;
	if( relai_state != (RELAI_PD_EN | RELAI_PD_CL | RELAI_M_EN | RELAI_M_CL | RELAI_P_EN | RELAI_P_CL | RELAI_40V | RELAI_PRECH) )
	{
		error("EMERGENC");
	}
	
	
	if( relai_volt < VCC_RELAI_MIN )
	{
		error("RELA MIN");
	}
	
	if( vcc_volt < VCC_MIN )
	{
		error("VCC  MIN");
	}
	
	// hv active
	if( (tmp = mcan_check()) )
	{
		switch(tmp)
		{
			case CAN_TRACTIVE_DISABLE: // CAN_CHARGE_ENABLE == 0x2FB
			{
				// disable hv
				standby();
			}
			break;
		}
	}
}

static inline void charge_loop(void)
{
	static unsigned int tmp = 0;
	if( relai_state != (RELAI_PD_EN | RELAI_PD_CL | RELAI_M_EN | RELAI_M_CL | RELAI_P_EN | RELAI_P_CL | RELAI_40V | RELAI_PRECH) )
	{
		error("RELAIOPE");
	}
	
	if( relai_volt < VCC_RELAI_MIN )
	{
		error("RELA MIN");
	}
	
	if( vcc_volt < VCC_MIN )
	{
		error("VCC  MIN");
	}
	
	/*if( relai_state != 0xFF && !(battery_state & STATE_INTERMEDIATE) )
	{
		// relais open
		if( (UCSR0A & (1<<UDRE)) )
		{
			if( charger_state == 0 )
			{
				charger_set_remote();
			}
			else if( charger_state == CHARGER_STATE_REMOTE )
			{
				charger_set_volt();
			}
			else if( charger_state == (CHARGER_STATE_REMOTE|CHARGER_STATE_VOLT_SET) )
			{
				charger_set_idle_curr();
			}
			else if( charger_state == CHARGER_IDLE )
			{
				charger_enable_output();
			}
			else if( charger_state == CHARGER_IDLE_ON )
			{
				battery_state |= STATE_INTERMEDIATE;
			}
		}
	}
	else if( relai_state == 0xFF )
	{
		// relais closed
		if( charger_state == CHARGER_IDLE_ON )
		{
			charger_set_charge_curr();
			charger_set_charge_curr();
			status("START CH");
		}
	}*/
	
	// charging
	if( (tmp = mcan_check()) )
	{
		switch(tmp)
		{
			case CAN_CHARGE_DISABLE: // CAN_CHARGE_ENABLE == 0x2FB
			{
				// disable hv
				standby();
			}
			break;
		}
	}
}

int main(void)
{
	static unsigned int tmp = 0;
	
	SET_OUTPUT(LED_R);
	SET_OUTPUT(LED_G);
	SET_OUTPUT(LED_B);
	RESET_LED(LED_R);
	RESET_LED(LED_G);
	RESET_LED(LED_B);
	
#ifdef DCDCCTRL
	SET_OUTPUT(DCDCCTRL);
	RESET(DCDCCTRL);
#endif
	
	battery_state = 0;
	
	slave_init();
	//uart_init();
	timer_init();
	mcan_init();
	spi_init();
	spi_adc_init();
	relai_init();
	adc_init();
	 
	sei();
	
	if( (GPIOR0 & (1<<WDRF)) == (1<<WDRF) )
	{
		status("WDTRST  ");
	}
	else if( (GPIOR0 & (1<<BORF)) == (1<<BORF) )
	{
		status("BORRST  ");
	}
	else if( (GPIOR0 & (1<<EXTRF)) == (1<<EXTRF) )
	{
		status("START EX");
	}
	else if( (GPIOR0 & (1<<PORF)) == (1<<PORF) )
	{
		status("START PO");
	}
	else if( (GPIOR0 & (1<<JTRF)) == (1<<JTRF) )
	{
		status("START JT");
	}
	else
	{
		status("START   ");
	}
	
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = (1<<WDE) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0);
	
	SET_LED(LED_G);
	
	while (1)
	{
		__asm__ __volatile__ ("wdr");
		
		if( battery_state & STATE_INTERMEDIATE )
		{
			intermediate_loop();
		}
		else if( battery_state & STATE_TRACTIVE )
		{
			tractive_loop();
		}
		else if( battery_state & STATE_CHARGING )
		{
			charge_loop();
		}
		else if( battery_state & STATE_BALANCING )
		{
			balancing_loop();
		}
		else
		{
			// standby mode
			if( (tmp = mcan_check()) && !(battery_state & STATE_ERROR) )
			{
				switch(tmp)
				{
					case CAN_TRACTIVE_ENABLE: // CAN_TRACTIVE_ENABLE == 0x2F0
					{
						if( relai_volt < VCC_RELAI_MIN || vcc_volt < VCC_MIN )
						{
							status("NO   VCC");
							break;
						}
						
						// enable hv
						intermediate_init();
						dspace_heartbeat = 0;
						battery_state |= STATE_TRACTIVE;
					}
					break;
					case CAN_CHARGE_ENABLE: // CAN_CHARGE_ENABLE == 0x2FA
					{
						if( relai_volt < VCC_RELAI_MIN || vcc_volt < VCC_MIN )
						{
							status("NO   VCC");
							break;
						}
						
						// start charging
						intermediate_init();
						battery_state |= STATE_CHARGING;
					}
					break;
					case CAN_BALANCING_ENABLE:
					{
						balancing_start();
						battery_state |= STATE_BALANCING;
					}
					break;
				}
			}
		}
		
		slave_loop();
		mcan_send_loop();
		adc_loop();
	}
	return 0;
}
