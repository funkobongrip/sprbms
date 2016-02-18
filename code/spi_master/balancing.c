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

volatile uint32_t balancing_times[NUMCELLS];
volatile uint32_t balancing_cycle_time;
volatile unsigned char balancing_send_times = 0;
volatile unsigned int balancing_min_cell_volt = 0xFFFF;

void balancing_start(void)
{
	if( slave_min_cell_volt < BALANCE_MIN_CELL_VOLT )
	{
		error("BAL  3.9");
		return;
	}
	
	if( battery_state != 0 )
	{
		error("BAL ONLY");
	}
	
	battery_state |= STATE_BALANCING;
	
	balancing_min_cell_volt = slave_min_cell_volt;
	
	balancing_calc_times();
}

void balancing_stop(void)
{
	battery_state &= ~STATE_BALANCING;
}

void balancing_loop(void)
{
	static unsigned int tmp = 0;
	if( (battery_state & STATE_BALANCING) && balancing_cycle_time == 0 )
	{
		balancing_calc_times();
	}
	
	// charging
	if( (tmp = mcan_check()) )
	{
		switch(tmp)
		{
			case CAN_BALANCING_DISABLE:
			{
				// disable hv
				standby();
			}
			break;
		}
	}
}

void balancing_set(void)
{
	static unsigned char on_time = 0;
	unsigned char i, balance_any = 0;
	
	// balancing outputs are always zerod, we only have to set them when we want to enable them
	if( on_time == 0 )
	{
		for(i=0; i<NUMCELLS ;i++)
		{
			if( cell_voltages[i] <= balancing_min_cell_volt )
			{
				balancing_times[i] = 0;
			}
			
			if( balancing_times[i] > 0 )
			{
				slave_set_balance_cell(i, 1);
				balance_any = 1;
			}
			else
			{
				slave_set_balance_cell(i, 0);
			}
		}
	}
	
	on_time++;
	if( on_time == 3 )
	{
		on_time = 0;
	}
	
	if( balance_any == 0 )
	{
		
	}
}

void balancing_calc_times(void)
{
	unsigned char i;
	unsigned int tmp = 0;
	for(i=0; i<NUMCELLS ;i++)
	{
		if( (cell_voltages[i] - balancing_min_cell_volt) > BALANCE_MIN_DIFF )
		{
			balancing_times[i] = (cell_voltages[i] - balancing_min_cell_volt) * BALANCE_TIME_FACTOR;
			if( balancing_times[i] > tmp )
				tmp = balancing_times[i];
		}
		else
		{
			balancing_times[i] = 0;
		}
	}
	balancing_send_times = 1;
	balancing_cycle_time = tmp + BALANCE_WAIT_CYCLE;
}
