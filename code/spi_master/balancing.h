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
 
#ifndef BALANCING_H
#define	BALANCING_H

extern volatile uint32_t balancing_times[NUMCELLS];
extern volatile uint32_t balancing_cycle_time;
extern volatile unsigned char balancing_send_times;

void balancing_start(void);
void balancing_stop(void);
void balancing_loop(void);
void balancing_calc_times(void);
void balancing_set(void);

#endif	// BALANCING_H
