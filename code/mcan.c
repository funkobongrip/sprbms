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
#include "adc.h"

can_filter_t can_filters[] = {
		{
			.id = 0x005,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x2F0,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x2F1,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x2F2,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x2F3,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x2FA,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x2FB,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		},
		{
			.id = 0x310,
			.mask = 0x7FF,
			.flags =  {
				.rtr = 0,
				.extended = 0
			}
		}
	};
volatile can_t cmsg;
volatile can_t rcmsg;volatile can_error_register_t cerr;
volatile unsigned char can_cycle_ms = 0;
volatile unsigned char can_send_data = 0, can_send_error = 0, can_send_status = 0;
volatile unsigned int dspace_heartbeat = 0;
volatile unsigned int dspace_dc_link_voltage = 0;
volatile unsigned int mcan_send_next = 0;
char etmp[9];int can_putchar(char c, FILE *stream);static FILE canstdout = FDEV_SETUP_STREAM(can_putchar, NULL, _FDEV_SETUP_WRITE); void mcan_init(void){
	unsigned char i;
	
	can_init(BITRATE_500_KBPS);

	for(i=0; i<8 ;i++)
	{
		can_set_filter(i, &can_filters[i]);
	}

	// Create a basic message
	cmsg.id = 0x123;
	cmsg.flags.rtr = 0;
	cmsg.flags.extended = 0;

	cmsg.length = 1;
	cmsg.data[0] = 0xab;	stdout = &canstdout;
}int can_putchar(char c, FILE *stream){	cmsg.id = 0x3FF;
	cmsg.length = 1;	cmsg.data[0] = c;	can_send_message((const can_t *)&cmsg);	while(! can_check_free_buffer())	{}	_delay_us(500);	return 0;}

void mcan_send_message(unsigned int id, unsigned char len, char *data)
{
	cmsg.id = id;
	cmsg.length = len;
	if( len > 0 )
		memcpy((void *)cmsg.data, data, len);
	can_send_message((const can_t *)&cmsg);
}

unsigned char mcan_send_loop(void)
{
	static unsigned int tmp = 0;
	static unsigned char can_current = 0;
	
	if( !can_check_free_buffer() )
	{
		return 0;
	}
	
	if( mcan_send_next == 0 )
	{
		return 0;
	}
	else
	{
		mcan_send_next = 0;
	}
	
	if( can_send_status == 1 )
	{
		update_states();
		
		cmsg.id = CAN_STATE_ID;
		cmsg.length = 3;
		cmsg.data[0] = battery_state;
		cmsg.data[1] = relai_state;
		cmsg.data[2] = relai_ext_state;
		can_send_message((const can_t *)&cmsg);
		can_send_status = 0;
		
		if( (battery_state & STATE_ERROR) && can_send_error )
		{
			cmsg.id = CAN_ERROR_ID;
			cmsg.length = 8;
			memcpy((can_t *)cmsg.data, (char *)error_msg, 8);
			can_send_message((const can_t *)&cmsg);
			can_send_error = 0;
		}
		
		if( (battery_state & STATE_BALANCING) && (balancing_send_times > 0) )
		{
			static unsigned int num_send = 0;
			if( num_send == 0 )
			{
				sprintf(etmp, "%8d", balancing_send_times);
				status(etmp);
				num_send = 1;
			}
			else
			{
				sprintf(etmp, "%8lu", balancing_times[balancing_send_times]);
				status(etmp);
				num_send = 2;
			}
			
			if( balancing_send_times == (NUMCELLS-1) && num_send == 2 )
			{
				balancing_send_times = 0;
				sprintf(etmp, "t%7lu", balancing_cycle_time);
				status(etmp);
				num_send = 0;
			}
			else if( num_send == 2 )
			{
				balancing_send_times++;
				num_send = 0;
			}
		}
	}
	// check if data should be transmitted over the can bus, if so, do it
	else if( can_send_data )
	{
		if( can_current < 21 )
		{
			cmsg.id = CAN_BASE_CELL_ID + can_current;
			
			cmsg.length = 8;
			
			tmp = (can_current * 4);
						cmsg.data[0] = cell_voltages[tmp+0] & 0xFF;
			cmsg.data[1] = ((cell_voltages[tmp+0] >> 8) & 0x0F) | ((cell_voltages[tmp+1] << 4) & 0xF0);
			cmsg.data[2] = (cell_voltages[tmp+1] >> 4) & 0xFF;
			
			cmsg.data[3] = cell_voltages[tmp+2] & 0xFF;
			cmsg.data[4] = ((cell_voltages[tmp+2] >> 8) & 0x0F) | ((cell_voltages[tmp+3] << 4) & 0xF0);
			cmsg.data[5] = (cell_voltages[tmp+3] >> 4) & 0xFF;
			
			tmp = (can_current * 2);
			cmsg.data[6] = cell_temps[tmp+0];
			cmsg.data[7] = cell_temps[tmp+1];
			
			can_send_message((const can_t *)&cmsg);
		}
		else if( can_current == 21 )
		{
			cmsg.id = CAN_FULL_FSM_ID;
			memcpy((char *)cmsg.data, (void *)&batt_fsm_volt_copy, 4);
			cmsg.length = 4;
			can_send_message((const can_t *)&cmsg);
		}
		else if( can_current == 22 )
		{
			cmsg.id = CAN_OUTPUT_ADC;
			memcpy((char *)cmsg.data, (void *)&adc_in, 2);
			cmsg.length = 2;
			can_send_message((const can_t *)&cmsg);
		}
		else if( can_current == 23 )
		{
			cmsg.id = CAN_CELL_MIN_MAX_AVG;
			cmsg.data[0] = slave_max_cell;
			cmsg.data[1] = slave_min_cell;
			cmsg.data[2] = slave_max_cell_volt  & 0xFF;
			cmsg.data[3] = ((slave_min_cell_volt << 4) & 0xF0) | ((slave_max_cell_volt >> 8) & 0x0F);
			cmsg.data[4] = (slave_min_cell_volt >> 4) & 0xFF;
			cmsg.data[5] = slave_avg_cell_volt & 0xFF;
			cmsg.data[6] = (slave_avg_cell_volt >> 8) & 0x0F;
			cmsg.length = 7;
			can_send_message((const can_t *)&cmsg);
		}
		else if( can_current == 24 )
		{
			cmsg.id = CAN_TEMP_MIN_MAX_AVG;
			cmsg.data[0] = slave_max_temp;
			cmsg.data[1] = slave_min_temp;
			cmsg.data[2] = slave_max_temp_val;
			cmsg.data[3] = slave_min_temp_val;
			cmsg.data[4] = slave_avg_temp_val;
			cmsg.length = 5;
			can_send_message((const can_t *)&cmsg);
		}
		else if( can_current == 25 )
		{
			cmsg.id = CAN_LV_MEASURE;
			cmsg.data[0] = vcc_volt;
			cmsg.data[1] = relai_volt;
			cmsg.length = 2;
			can_send_message((const can_t *)&cmsg);
		}
		else
		{
			can_send_data = 0;
			can_current = 0;
			return 1;
		}
		can_current++;
		return 1;
	}
	return 0;
}
unsigned int mcan_check(void){	// Check if a new messag was received
	if( can_check_message() )
	{
		// Try to read the message
		if( can_get_message((can_t *)&rcmsg) )
		{
			if( rcmsg.id == 0x005 && rcmsg.length == 2 && rcmsg.data[0] == 0x13 && rcmsg.data[1] == 0x24 )
			{
				WDTCR = (1<<WDCE) | (1<<WDE);
				WDTCR = (1<<WDE);
				while(1) {}
			}
			else if( rcmsg.id == CAN_DSPACE_DC_LINK_VOLTAGE )
			{
				dspace_dc_link_voltage = rcmsg.data[0];
				dspace_dc_link_voltage |= (rcmsg.data[1] << 8);
				dspace_heartbeat = 0;
			}
			else
			{
				return rcmsg.id;
			}
		}
		
	}	return 0;}