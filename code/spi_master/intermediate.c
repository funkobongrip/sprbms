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

volatile uint16_t precharge_time = 1;
volatile unsigned char precharge_seen = 0;

void intermediate_init(void)
{
		precharge_time = 1;
		precharge_seen = 0;
		SET_LED(LED_R);
		battery_state |= STATE_INTERMEDIATE;
}

// this file contains the loops that maintain the precharge/discharge cycles
unsigned int intermediate_loop(void)
{
	if( relai_volt < VCC_RELAI_MIN )
	{
		error("RELA MIN");
	}
	
	if( vcc_volt < VCC_MIN )
	{
		error("VCC  MIN");
	}
	
	// precharge active
	if( battery_state & STATE_CHARGING )
	{
		// there should always be relai voltage available
		if( !IS_SET(RELAI_VCC) )
		{
			error("RELAIVCC");
			return 1;
		}
		// check maximum time
		else if( precharge_time > PRECHARGE_CHARGE_MAX )
		{
			error("PRECHTIM");
			return 1;
		}
		// close precharge relai at defined time
		else if( !(relai_state & RELAI_PD_EN) && precharge_time > PRECHARGE_CHARGE_CC )
		{
			// TODO: check spi adc measurement here
			// first close precharge relai
			SET_RELAI(RELAICHARGE_CTRL);
			relai_state |= RELAI_PD_EN;
			status("CHARGE C");
			return 1;
		}
		// close minus relai at defined time
		else if( !(relai_state & RELAI_M_EN) && precharge_time > PRECHARGE_CHARGE_MC )
		{
			// close - relai
			SET_RELAI(RELAIMINUS_CTRL);
			relai_state |= RELAI_M_EN;
			status("MINUS  C");
			return 1;
		}
		// all bits on means everything good, precharge done!
		else if ( relai_state == RELAI_STATE_READY )
		{
			// looks fine, leave intermediate mode
			status("PRECHDON");
			mcan_send_message(CAN_PRECHARGE_DONE_ID, 0, NULL);
			// send CAN_PRECHARGE_DONE_ID
			battery_state &= ~STATE_INTERMEDIATE;
			return 1;
		}
		// close plus relai if there is no current
		else if ( relai_state == RELAI_STATE_PRECHARGE_DONE && precharge_time > PRECHARGE_CHARGE_PC )
		{
			// TODO check spi measurement here
			SET_RELAI(RELAIPLUS_CTRL);
			relai_state |= RELAI_P_EN;
			status("PLUS   C");
			return 1;
			// todo: timer based waiting
		}
	}
	else if( battery_state & STATE_TRACTIVE )
	{
		// TODO: check if any relai is closed already!
		// precharge should not take longer than fifteen seconds
		if( !IS_SET(RELAI_VCC) )
		{
			error("RELAIVCC");
			return 1;
		}
		else if( precharge_time > PRECHARGE_TRACTIVE_MAX )
		{
			error("PRECHTIM");
			return 1;
		}
		else if( !(relai_state & RELAI_PD_EN) && precharge_time > PRECHARGE_TRACTIVE_CC )
		{
			// TODO: check spi adc measurement here
			// there should be no voltage at all now
			if(! IS_SET(N40V) )
			{
				error("PRE B 40");
				return 1;
			}
			// first close precharge relai
			SET_RELAI(RELAICHARGE_CTRL);
			relai_state |= RELAI_PD_EN;
			status("CHARGE T");
			return 1;
		}
		else if( !(relai_state & RELAI_M_EN) && precharge_time > PRECHARGE_CHARGE_MC )
		{
			// there should be no voltage at all now
			if(! IS_SET(N40V) )
			{
				error("PRE B 40");
				return 1;
			}
			// close - relai
			SET_RELAI(RELAIMINUS_CTRL);
			relai_state |= RELAI_M_EN;
			status("MINUS  T");
			return 1;
		}
		else if ( relai_state == RELAI_STATE_READY )
		{
			// send CAN_PRECHARGE_DONE_ID
			// looks fine, leave intermediate mode
			status("PRECHDON");
			mcan_send_message(CAN_PRECHARGE_DONE_ID, 0, NULL);
#ifdef DCDCCTRL
			SET(DCDCCTRL);
#endif
			battery_state &= ~STATE_INTERMEDIATE;
			return 1;
		}
		else if ( relai_state != RELAI_STATE_PRECHARGING && precharge_time == PRECHARGE_TRACTIVE_PRECHARGE )
		{
			error("NO PRECH");
			return 1;
		}
		else if ( relai_state == RELAI_STATE_PRECHARGE_DONE && precharge_time < PRECHARGE_TRACTIVE_MINTIME )
		{
			error("PRE FAST");
			return 1;
		}
		else if (relai_state == RELAI_STATE_PRECHARGE_DONE && precharge_time > PRECHARGE_TRACTIVE_PC )
		{
			// TODO check spi measurement here
			SET_RELAI(RELAIPLUS_CTRL);
			relai_state |= RELAI_P_EN;
			status("PLUS   T");
			return 1;
		}
	}
	
	static unsigned char tmp;
	
	if( (tmp = mcan_check()) )
	{
		// TODO: no messages in this state
		if( tmp == CAN_TRACTIVE_DISABLE || tmp == CAN_CHARGE_DISABLE )
		{
			status("IRQ DISC");
			standby();
			return 1;
		}
	}
	return 0;
}
