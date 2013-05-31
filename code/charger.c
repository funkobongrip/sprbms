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

#define		MAX_VOLTAGE		"352.8"
#define		IDLE_CURR		"0.01"
#define		BALANCE_CURR	"0.2"
#define		CHARGE_CURR		"24"

/*
commands:
GTR - remote control enable
MODE,UI - set Mode
UA,100.0 - set output voltage
IA,10.0 - set output current
MU - display current voltage
MI - display current current
STATUS - get status byte
SB,R - disable output
SB,S - enable output
*/

unsigned char charger_state = 0;
unsigned char charger_sent_command = 0;

void charger_set_remote(void)
{
	uart_str("GTR\r\nMODE,UI\r\nSB,S\r\n");
	//uart_send_str("GTR\r\nMODE,UI\r\nSB,S\r\n");
	charger_state = CHARGER_STATE_REMOTE;
}

void charger_set_volt(void)
{
	uart_str("UA," MAX_VOLTAGE "\r\n");
	//uart_send_str("UA," MAX_VOLTAGE "\r\n");
	charger_state |= CHARGER_STATE_VOLT_SET;
}

void charger_set_idle_curr(void)
{
	uart_str("IA," IDLE_CURR "\r\n");
	//uart_send_str("IA,0.01\r\n");
	charger_state &= ~CHARGER_STATE_CHARGE_CURR;
	charger_state &= ~CHARGER_STATE_BALANCE_CURR;
	charger_state |= CHARGER_STATE_IDLE_CURR;
}

void charger_enable_output(void)
{
	uart_str("SB,R\r\n");
	charger_state |= CHARGER_STATE_OUTPUT;
}

void charger_disable_output(void)
{
	uart_str("SB,S\r\n");
	charger_state &= ~CHARGER_STATE_OUTPUT;
}

void charger_set_charge_curr(void)
{
	uart_str("IA," CHARGE_CURR "\r\n");
	//uart_send_str("IA,0.01\r\n");
	charger_state &= ~CHARGER_STATE_IDLE_CURR;
	charger_state &= ~CHARGER_STATE_BALANCE_CURR;
	charger_state |= CHARGER_STATE_CHARGE_CURR;
}

void charger_set_balance_curr(void)
{
	uart_str("IA," BALANCE_CURR "\r\n");
	//uart_send_str("IA,0.01\r\n");
	charger_state &= ~CHARGER_STATE_IDLE_CURR;
	charger_state &= ~CHARGER_STATE_CHARGE_CURR;
	charger_state |= CHARGER_STATE_BALANCE_CURR;
}

void charger_set_current(unsigned int curr)
{
	static char currstr[16];
	sprintf(currstr, "IA,%u.0\r\n", curr);
	uart_str(currstr);
}
