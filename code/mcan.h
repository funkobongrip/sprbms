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

#ifndef MCAN_H
#define	MCAN_H

#include "can.h"

extern volatile can_t cmsg;
extern volatile unsigned char can_cycle_ms;
extern volatile unsigned char can_send_data, can_send_status, can_send_error;
extern volatile unsigned int dspace_dc_link_voltage;
extern volatile unsigned int dspace_heartbeat;
extern volatile unsigned int mcan_send_next;
extern char etmp[9];

void mcan_init(void);
unsigned int mcan_check(void);
unsigned char mcan_send_loop(void);
void mcan_send_message(unsigned int id, unsigned char len, char *data);#endif	// MCAN_H
