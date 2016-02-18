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
 
#ifndef CHARGER_H
#define	CHARGER_H

enum {
	CHARGER_STATE_REMOTE = 0x01,
	CHARGER_STATE_VOLT_SET = 0x02,
	CHARGER_STATE_IDLE_CURR = 0x04,
	CHARGER_STATE_BALANCE_CURR = 0x08,
	CHARGER_STATE_CHARGE_CURR = 0x10,
	CHARGER_STATE_OUTPUT = 0x20
};

#define CHARGER_IDLE		(CHARGER_STATE_REMOTE|CHARGER_STATE_VOLT_SET|CHARGER_STATE_IDLE_CURR)
#define CHARGER_IDLE_ON		(CHARGER_STATE_REMOTE|CHARGER_STATE_VOLT_SET|CHARGER_STATE_IDLE_CURR|CHARGER_STATE_OUTPUT)
#define CHARGER_BALANCING_ON		(CHARGER_STATE_REMOTE|CHARGER_STATE_VOLT_SET|CHARGER_STATE_BALANCE_CURR|CHARGER_STATE_OUTPUT)
#define CHARGER_CHARGE_ON		(CHARGER_STATE_REMOTE|CHARGER_STATE_VOLT_SET|CHARGER_STATE_CHARGE_CURR|CHARGER_STATE_OUTPUT)

#define charging()		((relai_state == 0xFF) && !(battery_state & STATE_INTERMEDIATE) && (battery_state & STATE_CHARGING))

extern unsigned char charger_state, charger_sent_command;

void charger_set_remote(void);
void charger_set_volt(void);
void charger_set_idle_curr(void);
void charger_enable_output(void);
void charger_disable_output(void);
void charger_set_charge_curr(void);
void charger_set_balance_curr(void);
void charger_set_current(unsigned int curr);#endif	// CHARGER_H
