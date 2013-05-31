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

#include "config.h"
#include "timer.h"
#include "main.h"
#include "slave.h"
#include "intermediate.h"
#include "mcan.h"
#include "balancing.h"
#include "adc.h"

unsigned int milliseconds = 0;
unsigned int seconds = 0;
unsigned int hours = 0;

void timer_init(void){	TCCR1B = (1 << CS11);	TCNT1H = 0xF8;
	TCNT1L = 0x30;	TIMSK1 = (1 << TOIE1);}ISR(TIMER1_OVF_vect) {
	char cSREG;
	cSREG = SREG;
	cli();
	
	TCNT1H = 0xF8;
	TCNT1L = 0x30;
	
	if( milliseconds == 999 )
	{
		milliseconds = 0;
		seconds++;
		if( seconds == 3599 )
		{
			hours++;
			if( hours == 0xFFFF )
			{
				// hours overflowed, this should NEVER happen
				// if it does, kill the bms.
				error("TIMESEND");
			}
		}
		
		if( battery_state & STATE_BALANCING )
		{
			unsigned char i;
			
			if( balancing_cycle_time > 0)
				balancing_cycle_time--;
				
			for(i=0; i<NUMCELLS ;i++)
			{
				if( balancing_times[i] > 0 )
					balancing_times[i]--;
			}
		}
		
		if( warning_time_val > 0 )
		{
			warning_time_val--;
		}
		else
		{
			battery_state &= ~STATE_WARNING;
		}
		
		
		can_send_error = 1;
	}
	else
	{
		milliseconds++;
	}
	
	if( (milliseconds % 50) == 0 )
		can_send_status = 1;
	
	// check cycle time of slave module
	slave_cycle_ms++;
	if( slave_cycle_ms == SLAVE_CYCLE_WARN_MS )
	{
		if( (battery_state & STATE_WARNING) == 0 )
		{
			status("WARN SLT");
		}
		warning_time(SLAVE_CYCLE_WARN_TIME);
	}
	
	if( slave_cycle_ms == SLAVE_CYCLE_MAX_MS )
	{
		if( !(battery_state & STATE_ERROR) )
		{
			error("NO MEASU");
		}
	}
	
	if( battery_state & STATE_TRACTIVE )
	{
		dspace_heartbeat++;
		if( dspace_heartbeat > DSPACE_HEARTBEAT_MS )
		{
			error("MAB   HB");
		}
	}
	
	if( slave_adc_time > 0 )
		slave_adc_time--;
	
	if( slave_uv_time > 0 )
		slave_uv_time++;
	
	if( slave_ov_time > 0 )
		slave_ov_time++;
	
	if( slave_ot_time > 0 )
		slave_ot_time++;
	
	if( precharge_time > 0 )
		precharge_time++;
		
	if( adc_wait_timer > 0 )
		adc_wait_timer--;
	
	mcan_send_next = 1;
	
	sei();
	SREG = cSREG;}
